#include <sound.h>

/*
static void staticSoundCallback(void *ref) {
	static_cast<SoundMixer*>(ref)->soundCallback();
}*/

void SoundMixer::soundCallback() {
	unsigned int out = 0;
	for (SoundChNum i = 0; i < chCount; i++) {
		if (chActive[i]) {
			SoundInfo_t sound = chSound[i];
			out += sound.data[chPos[i]++] * chVolume[i];
			if (chPos[i] >= sound.len) {
				if (sound.repeat) {
					chPos[i] = 0;
				} else {
					stop(i);
				}
			}
		}
	}
	out = out / 255 / chCount;
	dac_output_voltage(dacCh, min(out, 255)); // Do NOT overload
	if (rand() % 1000 == 0) Serial.println(out);
}

void SoundMixer::incSound() {
	if (chActiveCount == 0) esp_timer_start_periodic(timer, delay); // Logic!
	chActiveCount++;
}

void SoundMixer::decSound() {
	chActiveCount--;
	if (chActiveCount == 0) {
		esp_timer_stop(timer); // More logic.
		dac_output_voltage(dacCh, 0); // Turn off output
	}
}

SoundMixer::SoundMixer(SoundChNum channels, dac_channel_t dac, unsigned int frequency) {
	assert(channels <= CONFIG_SND_MAX_CHANNELS);
	chCount = channels;
	dacCh = dac;
	delay = 1000000 / frequency;

	dac_output_enable(dacCh);
	esp_timer_create_args_t timer_args;

	timer_args.callback = reinterpret_cast<esp_timer_cb_t>(&SoundMixer::soundCallback);
	timer_args.arg = this;
	timer_args.dispatch_method = ESP_TIMER_TASK;
	timer_args.name = "Sound timer";

	esp_timer_create(&timer_args, &timer);

	for (SoundChNum i = 0; i < channels; i++) { // Set defaults
		chActive[i] = false;
		chPaused[i] = false;
		chVolume[i] = 255;
	}
}

SoundMixer::~SoundMixer() {
	esp_timer_stop(timer);
	esp_timer_delete(timer);
}

void SoundMixer::play(SoundChNum channel, SoundInfo_t sound) {
	stop(channel);
	chSound[channel] = sound;
	chPos[channel] = 0;
	chActive[channel] = true;
	incSound();
}

bool SoundMixer::stop(SoundChNum channel) {
	if (chActive[channel] || chPaused[channel]) {
		chActive[channel] = false;
		chPaused[channel] = false;
		decSound();
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
		chActive[channel] = false;
		chPaused[channel] = true;
		decSound();
	}
	return true;
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
		chActive[channel] = true;
		chPaused[channel] = false;
		incSound();
	}
	return true;
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
