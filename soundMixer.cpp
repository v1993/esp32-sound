#include <sound.h>

static int gcd(int a, int b) {
	while(true) {
		if (a == 0) return b;
		b %= a;
		if (b == 0) return a;
		a %= b;
	}
}

static int lcm(int a, int b) {
	int temp = gcd(a, b);

	return temp ? (a / temp * b) : 0;
}

bool SoundMixer::handleQueue() {
	xSemaphoreTake(mutex, portMAX_DELAY);
	SoundControl_t ctrl;
	bool upd = false; // Should we recalculate anything?
	while(xQueueReceive(queue, &ctrl, 0) == pdTRUE) { // Handle all events without blocking
		SoundChNum channel = ctrl.channel;
		SoundProvider *sound;
		if (ctrl.event == START) {
			sound = ctrl.provider;
		} else {
			sound = chSound[channel];
		}
		SoundVolume vol = ctrl.vol;
		switch(ctrl.event) {
			case STOP:
				if (chActive[channel]) {upd = true; decSound();}
				if (chActive[channel] || chPaused[channel]) {
						chActive[channel] = false;
						chPaused[channel] = false;
						sound->provider_stop();
				}
				break;
			case START: // I'm sure that channel is free
				upd = true;
				incSound();
				chSound[channel] = sound;
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
				chVolume[channel] = vol;
				break;
		}
	}
	xSemaphoreGive(mutex);
	return upd;
}

void SoundMixer::setupTimer() {
	SoundChNum activeCount = uxSemaphoreGetCount(chActiveCount);
	if (activeCount == 1) { // Only one sound
		for (SoundChNum i = 0; i < chCount; i++) { if (chActive[i]) {
			chSound[i]->divisor = 1;
			esp_timer_start_periodic(timer, SOUND_FREQ_TO_DELAY(chSound[i]->getFrequency()));
			break;
		}}
		counterMax = 1;
	} else {
		SoundChNum n = 0;
		unsigned long int freqArr[activeCount];
		for (SoundChNum i = 0; i < chCount; i++) { if (chActive[i]) {
			freqArr[n++] = chSound[i]->getFrequency();
		}}

		int freqLcm = std::accumulate(&(freqArr[1]), &(freqArr[activeCount]), freqArr[0], lcm);
		for (SoundChNum i = 0; i < chCount; i++) { if (chActive[i]) {
			SoundProvider *sound = chSound[i];
			sound->divisor = freqLcm / sound->getFrequency();
		}}
		esp_timer_start_periodic(timer, SOUND_FREQ_TO_DELAY(freqLcm));
	}
}

void SoundMixer::soundCallback() {
	bool upd = handleQueue();
	if (upd) {
		esp_timer_stop(timer); // It will work OK anyway
		if (uxSemaphoreGetCount(chActiveCount) == 0) { // If nothing to play
			xSemaphoreGive(timerMutex);
			return;
		}
		setupTimer(); // TODO: implement that function
		counter = 0; // Only for later ++
	}

	counter++;
	if (counter > counterMax) counter = 1;

	unsigned int out = 0;
	for (SoundChNum i = 0; i < chCount; i++) { if (chActive[i]) {
		SoundProvider *sound = chSound[i];
		if ((counter % sound->divisor) == 0) {
			SoundData sample;
			if (xQueueReceive(sound->queue, &sample, 0) == pdTRUE) {
				sound->actual = sample;
			} 
		}
		out += sound->actual;
		SoundProviderControl_t ctrl;
		while(xQueueReceive(sound->controlQueue, &ctrl, 0) == pdTRUE) {
			switch(ctrl) {
				case END:
					if (sound->repeat) {
						sound->provider_restart();
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
}

void SoundMixer::incSound() {
	xSemaphoreGive(chActiveCount);
}

void SoundMixer::decSound() {
	xSemaphoreTake(chActiveCount, portMAX_DELAY);
}

void SoundMixer::addEvent(SoundControl_t event) {
	xQueueSendToBack(queue, &event, portMAX_DELAY);
}

SoundMixer::SoundMixer(SoundChNum normal_channels, SoundChNum auto_channels, dac_channel_t dac) {
	chCount = normal_channels + auto_channels;
	chFirstAuto = normal_channels; // It isn't mistake, but looks strange
	assert(chCount <= CONFIG_SND_MAX_CHANNELS);
	dacCh = dac;

	dac_output_enable(dacCh);
	esp_timer_create_args_t timer_args;

	timer_args.callback = reinterpret_cast<esp_timer_cb_t>(&SoundMixer::soundCallback);
	timer_args.arg = this;
	timer_args.dispatch_method = ESP_TIMER_TASK;
	timer_args.name = "Sound timer";

	esp_timer_create(&timer_args, &timer);

	mutex = xSemaphoreCreateMutex();
	timerMutex = xSemaphoreCreateCounting(1, 1);
	chActiveCount = xSemaphoreCreateCounting(chCount, 0);
	queue = xQueueCreate(CONFIG_SND_CONTROL_QUEUE_SIZE, sizeof(SoundControl_t));

	for (SoundChNum i = 0; i < chCount; i++) { // Set defaults
		chActive[i] = false;
		chPaused[i] = false;
		chVolume[i] = 255;
		chSound[i] = NULL;
	}
}

SoundMixer::~SoundMixer() {
	esp_timer_stop(timer);
	esp_timer_delete(timer);

	vSemaphoreDelete(mutex);
	vSemaphoreDelete(timerMutex);
	vSemaphoreDelete(chActiveCount);
	vQueueDelete(queue);
}

void SoundMixer::checkTimer() {
	if (xSemaphoreTake(timerMutex, 0) == pdTRUE) { // If timer isn't active
		esp_timer_start_once(timer, 0); // Activate one-shot handler
	}
}

void SoundMixer::play(SoundChNum channel, SoundProvider *sound) {
	stop(channel);

	SoundControl_t ctrl;
	ctrl.event = START;
	ctrl.channel = channel;
	ctrl.provider = sound;
	addEvent(ctrl);

	checkTimer();
}

void SoundMixer::stop(SoundChNum channel) {
	if (uxSemaphoreGetCount(timerMutex) == 0) {
		SoundControl_t ctrl;
		ctrl.event = STOP;
		ctrl.channel = channel;
		addEvent(ctrl);
	}
}

void SoundMixer::stopAll() {
	for (SoundChNum i = 0; i < chCount; i++) {
		stop(i);
	}
}

void SoundMixer::pause(SoundChNum channel) {
	if (uxSemaphoreGetCount(timerMutex) == 0) {
		SoundControl_t ctrl;
		ctrl.event = PAUSE;
		ctrl.channel = channel;
		addEvent(ctrl);
	}
}

void SoundMixer::pauseAll() {
	for (SoundChNum i = 0; i < chCount; i++) {
		pause(i);
	}
}

void SoundMixer::resume(SoundChNum channel) {
	SoundControl_t ctrl;
	ctrl.event = RESUME;
	ctrl.channel = channel;
	addEvent(ctrl);

	checkTimer();
}


void SoundMixer::resumeAll() {
	for (SoundChNum i = 0; i < chCount; i++) {
		resume(i);
	}
}

void SoundMixer::setVolume(SoundChNum channel, SoundVolume vol) {
	SoundControl_t ctrl;
	ctrl.event = VOLSET;
	ctrl.channel = channel;
	ctrl.vol = vol;
	addEvent(ctrl); // We don't call checkTimer because this event can be handled later
}

SoundVolume SoundMixer::getVolume(SoundChNum channel) {
	SoundVolume vol;
	xSemaphoreTake(mutex, portMAX_DELAY);
	vol = chVolume[channel];
	xSemaphoreGive(mutex);
	return vol;
}

SoundState_t SoundMixer::state(SoundChNum channel) {
	xSemaphoreTake(mutex, portMAX_DELAY);
	SoundState_t s;
	if (chActive[channel]) s = PLAYING;
	else if (chPaused[channel]) s = PAUSED;
	else s = STOPPED;
	xSemaphoreGive(mutex);
	return s;
}

SoundChNum SoundMixer::playAuto(SoundProvider *sound, SoundVolume vol) {
	for (SoundChNum i = chFirstAuto; i < chCount; i++) {
		if (state(i) == STOPPED) { // We found free channel, setting up
			setVolume(i, vol);
			play(i, sound);
			return i;
		}
	}
	return chCount; // No free channels
}
