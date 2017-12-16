#ifndef SOUND_H
#define SOUND_H

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

#include <stdint.h>
typedef unsigned int SoundChNum;
typedef uint8_t SoundData;
typedef long unsigned int SoundPos;
typedef unsigned int SoundVolume;

typedef struct SoundInfo {
	const SoundData *data;
	SoundPos len;
	bool repeat;
} SoundInfo_t;

typedef enum SoundState {
	STOPPED,
	PLAYING,
	PAUSED
} SoundState_t;

#include <esp_timer.h>
#include <driver/gpio.h>
#include <driver/dac.h>

#include <Arduino.h>

class SoundMixer {
	protected:
		esp_timer_handle_t timer = NULL; // Timer for this instance
		dac_channel_t dacCh; // DAC channel for sound

		unsigned int delay;
		SoundChNum chCount; // Total channels count, <= CONFIG_SND_MAX_CHANNELS
		SoundChNum chFirstAuto; // Number of first "auto" channel
		SoundChNum chActiveCount = 0; // Count of active channels (to control timer)
		SoundInfo_t chSound[CONFIG_SND_MAX_CHANNELS]; // Configuration of sound
		SoundPos chPos[CONFIG_SND_MAX_CHANNELS]; // Current position of music on channel
		bool chActive[CONFIG_SND_MAX_CHANNELS]; // Active channels
		bool chPaused[CONFIG_SND_MAX_CHANNELS]; // Paused channels

		void incSound(); // Increment counter and start timer (if isn't started yed)
		void decSound(); // Decrement counter and stop timer (if nothing to play anymore)

		void soundCallback(); // Play one step
	public:
		SoundVolume chVolume[CONFIG_SND_MAX_CHANNELS]; // Volume map

		SoundMixer(SoundChNum normal_channels, SoundChNum auto_channels, dac_channel_t dac, unsigned int frequency); // Setup SoundMixer
		~SoundMixer(); // Destroy timer

		void play(SoundChNum channel, SoundInfo_t sound);
		bool stop(SoundChNum channel);
		bool pause(SoundChNum channel);
		bool resume(SoundChNum channel);
		SoundState_t state(SoundChNum channel);

		SoundChNum playAuto(SoundInfo_t sound, SoundVolume vol); // Auto select channel and play sound on it (if no aviable, count of channels will be returned)

		bool stopAll();
		bool pauseAll();
		bool resumeAll();
};

#endif
