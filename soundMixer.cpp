#include "soundMixer.h"
#include "soundProvider.h"

#define SOUND_FREQ_TO_DELAY(f) (1000000/f)
#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

static int IRAM_ATTR gcd(int a, int b) {
	while(true) {
		if (a == 0) return b;
		b %= a;
		if (b == 0) return a;
		a %= b;
	}
}

static int IRAM_ATTR lcm(int a, int b) {
	int temp = gcd(a, b);

	return temp ? (a / temp * b) : 0;
}

namespace Sound {
	bool IRAM_ATTR SoundMixer::handleQueue() {
		SoundControl ctrl;
		bool upd = false; // Should we recalculate anything?
		std::lock_guard<std::mutex> queueLock(queueMutex);
		while(not queue.empty()) { // Handle all events without blocking
			SoundControl ctrl = queue.front();
			queue.pop();
			SoundChNum channel = ctrl.channel;
			if (ctrl.event == START) {
				chSound[channel] = std::move(ctrl.provider);
			}
			std::shared_ptr<SoundProvider>& sound = chSound[channel];
			switch(ctrl.event) {
				case STOP:
					if (chActive[channel]) {upd = true; decSound();}
					if (chActive[channel] || chPaused[channel]) {
							chActive[channel] = false;
							chPaused[channel] = false;
							sound->provider_stop();
							chSound[channel] = nullptr; // Release pointer
					}
					break;
				case START: // I'm sure that channel is free
					upd = true;
					incSound();
					chActive[channel] = true;
					sound->provider_start();
					sound->actual = 0;
					break;
				case PAUSE:
					if (chActive[channel]) {
							upd = true;
							decSound();
							chActive[channel] = false;
							chPaused[channel] = true;
							sound->provider_pause();
					}
					break;
				case RESTART:
					if (chActive[channel]) {
							sound->provider_restart();
					}
					break;
				case RESUME:
					if (chPaused[channel]) {
							upd = true;
							incSound();
							chActive[channel] = true;
							chPaused[channel] = false;
							sound->provider_resume();
					}
					break;
				case VOLSET:
					chVolume[channel] = ctrl.vol;
					break;
			}
		}
		return upd;
	}

	void IRAM_ATTR SoundMixer::setupTimer() {
		counterMax = 1;
		if (chActiveCount == 1) { // Only one sound
			for (SoundChNum i = 0; i < chCount; ++i) { if (chActive[i]) {
				chSound[i]->divisor = 1;
				esp_timer_start_periodic(timer, SOUND_FREQ_TO_DELAY(chSound[i]->getFrequency()));
				break;
			}}
		} else {
			SoundChNum n = 0;
			std::vector<unsigned long int> freqArr(chActiveCount);
			for (SoundChNum i = 0; i < chCount; ++i) { if (chActive[i]) {
				freqArr[n++] = chSound[i]->getFrequency();
			}}

			int freqLcm = std::accumulate(&(freqArr[1]), &(freqArr[chActiveCount]), freqArr[0], lcm);
			for (SoundChNum i = 0; i < chCount; ++i) { if (chActive[i]) {
				std::shared_ptr<SoundProvider> sound = chSound[i];
				sound->divisor = freqLcm / sound->getFrequency();
				counterMax = lcm(counterMax, sound->divisor);
			}}
			esp_timer_start_periodic(timer, SOUND_FREQ_TO_DELAY(freqLcm));
		}
	}

	void IRAM_ATTR SoundMixer::soundCallback() {
		std::unique_lock<std::shared_timed_mutex> lock(mutex);
		bool upd = handleQueue();
		if (upd) {
			esp_timer_stop(timer); // It will work OK anyway
			if (chActiveCount == 0) { // If nothing to play
				timerActive = false;
				dac_output_voltage(dacCh, 0); // Reduce energy usage
				return;
			}
			setupTimer();
			counter = 0; // Only for later ++
		}
		lock.unlock(); // We leave critical area

		++counter;
		if (counter > counterMax) counter = 1;

		unsigned int out = 0;
		upd = false;
		for (SoundChNum i = 0; i < chCount; ++i) { if (chActive[i]) {
			std::shared_ptr<SoundProvider>& sound = chSound[i];
			//if ((rand() % 1000) == 0) std::cout << sound.use_count() << std::endl;
			if ((counter % sound->divisor) == 0) {
				SoundData sample;
				if (xQueueReceive(sound->queue, &sample, 0) == pdTRUE) {
					sound->actual = sample;
				} 
			}
			out += sound->actual * chVolume[i];
			SoundProviderControl ctrl;
			while(xQueueReceive(sound->controlQueue, &ctrl, 0) == pdTRUE) {
				switch(ctrl) {
					case FREQUENCY_UPDATE:
						upd = true;
						break;
					case END:
						if (sound->repeat) {
							restart(i);
						} else {
							stop(i);
						}
						break;
					case FAILURE:
						stop(i);
						break;
				}
			}
		}}

		out = out / 255 / chCount;
		dac_output_voltage(dacCh, min(out, 255)); // Do NOT overload

		if (upd) { // If someone have changed frequency
			lock.lock();
			esp_timer_stop(timer);
			setupTimer();
			lock.unlock(); // If I'll add some code later
		}
	}

	void IRAM_ATTR SoundMixer::incSound() {
		++chActiveCount;
	}

	void IRAM_ATTR SoundMixer::decSound() {
		--chActiveCount;
	}

	void IRAM_ATTR SoundMixer::addEvent(const SoundControl& event) {
		std::lock_guard<std::mutex> queueLock(queueMutex);
		queue.push(event);
	}

	SoundMixer::SoundMixer(SoundChNum normal_channels, SoundChNum auto_channels, dac_channel_t dac):
	chCount(normal_channels + auto_channels),
	chFirstAuto(normal_channels) // It isn't mistake, but looks strange
	{
		assert(chCount <= CONFIG_SND_MAX_CHANNELS);
		dacCh = dac;

		dac_output_enable(dacCh);
		esp_timer_create_args_t timer_args;

		timer_args.callback = reinterpret_cast<esp_timer_cb_t>(&SoundMixer::soundCallback);
		timer_args.arg = this;
		timer_args.dispatch_method = ESP_TIMER_TASK;
		timer_args.name = "Sound timer";

		esp_timer_create(&timer_args, &timer);

		for (SoundChNum i = 0; i < chCount; ++i) { // Set defaults
			chActive[i] = false;
			chPaused[i] = false;
			chVolume[i] = 255;
			chSound[i] = nullptr;
		}
	}

	SoundMixer::~SoundMixer() {
		esp_timer_stop(timer);
		esp_timer_delete(timer);
	}

	void SoundMixer::checkTimer() {
		std::shared_lock<std::shared_timed_mutex> lock(mutex);
		if (not timerActive) { // If timer isn't active
			timerActive = true;
			esp_timer_start_once(timer, 0); // Activate one-shot handler
		}
	}

	void SoundMixer::play(SoundChNum channel, const std::shared_ptr<SoundProvider>& sound) {
		stop(channel);

		SoundControl ctrl;
		ctrl.event = START;
		ctrl.channel = channel;
		ctrl.provider = sound; // Copy
		addEvent(ctrl);

		checkTimer();
	}

	void SoundMixer::stop(SoundChNum channel) {
		std::shared_lock<std::shared_timed_mutex> lock(mutex);
		if (timerActive) {
			lock.unlock();
			SoundControl ctrl;
			ctrl.event = STOP;
			ctrl.channel = channel;
			addEvent(ctrl);
		}
	}

	void SoundMixer::stopAll() {
		for (SoundChNum i = 0; i < chCount; ++i) {
			stop(i);
		}
	}

	void SoundMixer::pause(SoundChNum channel) {
		std::shared_lock<std::shared_timed_mutex> lock(mutex);
		if (timerActive) {
			lock.unlock();
			SoundControl ctrl;
			ctrl.event = PAUSE;
			ctrl.channel = channel;
			addEvent(ctrl);
		}
	}

	void SoundMixer::pauseAll() {
		for (SoundChNum i = 0; i < chCount; ++i) {
			pause(i);
		}
	}

	void SoundMixer::restart(SoundChNum channel) {
		std::shared_lock<std::shared_timed_mutex> lock(mutex);
		if (timerActive) {
			SoundControl ctrl;
			ctrl.event = RESTART;
			ctrl.channel = channel;
			addEvent(ctrl);
		}
	}

	void SoundMixer::restartAll() {
		for (SoundChNum i = 0; i < chCount; ++i) {
			restart(i);
		}
	}

	void SoundMixer::resume(SoundChNum channel) {
		SoundControl ctrl;
		ctrl.event = RESUME;
		ctrl.channel = channel;
		addEvent(ctrl);

		checkTimer();
	}

	void SoundMixer::resumeAll() {
		for (SoundChNum i = 0; i < chCount; ++i) {
			resume(i);
		}
	}

	void SoundMixer::setVolume(SoundChNum channel, SoundVolume vol) {
		SoundControl ctrl;
		ctrl.event = VOLSET;
		ctrl.channel = channel;
		ctrl.vol = vol;
		addEvent(ctrl); // We don't call checkTimer because this event can be handled later
	}

	SoundVolume SoundMixer::getVolume(SoundChNum channel) {
		SoundVolume vol;
		std::shared_lock<std::shared_timed_mutex> lock(mutex);
		vol = chVolume[channel];
		return vol;
	}

	SoundState SoundMixer::state(SoundChNum channel) {
		std::shared_lock<std::shared_timed_mutex> lock(mutex);
		if (chActive[channel])      return PLAYING;
		else if (chPaused[channel]) return PAUSED;
		else                        return STOPPED;
	}

	SoundChNum SoundMixer::playAuto(const std::shared_ptr<SoundProvider>& sound, SoundVolume vol) {
		for (SoundChNum i = chFirstAuto; i < chCount; ++i) {
			if (state(i) == STOPPED) { // We found free channel, setting up
				setVolume(i, vol);
				play(i, sound);
				return i;
			}
		}
		return chCount; // No free channels
	}
}
