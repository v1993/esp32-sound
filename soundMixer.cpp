#include <sound.h>

#define ENTER_CRITICAL() critical++
#define LEAVE_CRITICAL() critical--

void SoundMixer::soundCallback() {
	if (critical > 0) return; // If we are in critical area, skip frame
	unsigned int out = 0;
	for (SoundChNum i = 0; i < chCount; i++) {
		if (chActive[i]) {
			SoundProvider *sound = chSound[i];
			out += sound->buf[sound->buf_pos++] * chVolume[i];
			if (sound->buf_pos == sound->buf_len) sound->buf_update(); // Grab more data
			if (sound->buf_len == 0) {
				if (sound->repeat) {
					sound->buf_reload();
				} else {
					stop(i);
				}
			}
		}
	}
	out = out / 255 / chCount;
	dac_output_voltage(dacCh, min(out, 255)); // Do NOT overload
	// if (rand() % 1000 == 0) Serial.println(out); // FIXME: delete it!
}

void SoundMixer::incSound() {
	ENTER_CRITICAL();
	if (chActiveCount == 0) esp_timer_start_periodic(timer, delay); // Logic!
	chActiveCount++;
	LEAVE_CRITICAL();
}

void SoundMixer::decSound() {
	ENTER_CRITICAL();
	chActiveCount--;
	if (chActiveCount == 0) {
		esp_timer_stop(timer); // More logic.
		dac_output_voltage(dacCh, 0); // Turn off output
	}
	LEAVE_CRITICAL();
}

SoundMixer::SoundMixer(SoundChNum normal_channels, SoundChNum auto_channels, dac_channel_t dac, unsigned int frequency) {
	chCount = normal_channels + auto_channels;
	chFirstAuto = normal_channels; // It isn't mistake, but looks strange
	assert(chCount <= CONFIG_SND_MAX_CHANNELS);
	dacCh = dac;
	delay = 1000000 / frequency;

	dac_output_enable(dacCh);
	esp_timer_create_args_t timer_args;

	timer_args.callback = reinterpret_cast<esp_timer_cb_t>(&SoundMixer::soundCallback);
	timer_args.arg = this;
	timer_args.dispatch_method = ESP_TIMER_TASK;
	timer_args.name = "Sound timer";

	esp_timer_create(&timer_args, &timer);

	for (SoundChNum i = 0; i < chCount; i++) { // Set defaults
		chActive[i] = false;
		chPaused[i] = false;
		chVolume[i] = 255;
	}
}

SoundMixer::~SoundMixer() {
	esp_timer_stop(timer);
	esp_timer_delete(timer);
}

void SoundMixer::play(SoundChNum channel, SoundProvider *sound) {
	ENTER_CRITICAL();
	stop(channel);
	sound->buf_init();
	chSound[channel] = sound;
	chPos[channel] = 0;
	chActive[channel] = true;
	incSound();
	LEAVE_CRITICAL();
}

bool SoundMixer::stop(SoundChNum channel) {
	if (chActive[channel] || chPaused[channel]) {
		ENTER_CRITICAL();
		chActive[channel] = false;
		chPaused[channel] = false;
		decSound();
		chSound[channel]->buf_deinit();
		LEAVE_CRITICAL();
		return true;
	}
	return false;
}

bool SoundMixer::stopAll() {
	bool res = false;
	for (SoundChNum i = 0; i < chCount; i++) {
		res |= stop(i);
	}
	return res;
}

bool SoundMixer::pause(SoundChNum channel) {
	if (chActive[channel]) {
		ENTER_CRITICAL();
		chActive[channel] = false;
		chPaused[channel] = true;
		decSound();
		LEAVE_CRITICAL();
		return true;
	}
	return false;
}

bool SoundMixer::pauseAll() {
	bool res = false;
	for (SoundChNum i = 0; i < chCount; i++) {
		res |= pause(i);
	}
	return res;
}

bool SoundMixer::resume(SoundChNum channel) {
	if (chPaused[channel]) {
		ENTER_CRITICAL();
		chActive[channel] = true;
		chPaused[channel] = false;
		incSound();
		LEAVE_CRITICAL();
		return true;
	}
	return false;
}


bool SoundMixer::resumeAll() {
	bool res = false;
	for (SoundChNum i = 0; i < chCount; i++) {
		res |= resume(i);
	}
	return res;
}

SoundState SoundMixer::state(SoundChNum channel) {
	if (chActive[channel]) return PLAYING;
	if (chPaused[channel]) return PAUSED;
	return STOPPED;
}

SoundChNum SoundMixer::playAuto(SoundProvider *sound, SoundVolume vol) {
	for (SoundChNum i = chFirstAuto; i < chCount; i++) {
		if (state(i) == STOPPED) { // We found free channel, setting up
			chVolume[i] = vol;
			play(i, sound);
			return i;
		}
	}
	return chCount; // No free channels
}
