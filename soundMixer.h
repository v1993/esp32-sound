#ifndef SOUND_MIXER_H
#define SOUND_MIXER_H

#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

#include <esp_timer.h>
#include <driver/gpio.h>
#include <driver/dac.h>

class SoundProvider;

class SoundMixer {
	protected:
		esp_timer_handle_t timer = NULL; // Timer for this instance
		int critical = 0;
		dac_channel_t dacCh; // DAC channel for sound

		unsigned int delay;
		SoundChNum chCount; // Total channels count, <= CONFIG_SND_MAX_CHANNELS
		SoundChNum chFirstAuto; // Number of first "auto" channel
		SoundChNum chActiveCount = 0; // Count of active channels (to control timer)
		SoundProvider* chSound[CONFIG_SND_MAX_CHANNELS]; // Sound provider pointers
		bool chActive[CONFIG_SND_MAX_CHANNELS]; // Active channels
		bool chPaused[CONFIG_SND_MAX_CHANNELS]; // Paused channels

		void incSound(); // Increment counter and start timer (if isn't started yed)
		void decSound(); // Decrement counter and stop timer (if nothing to play anymore)

		void soundCallback(); // Play one step
	public:
		SoundVolume chVolume[CONFIG_SND_MAX_CHANNELS]; // Volume map

		SoundMixer(SoundChNum normal_channels, SoundChNum auto_channels, dac_channel_t dac, unsigned int frequency); // Setup SoundMixer
		~SoundMixer(); // Destroy timer

		void play(SoundChNum channel, SoundProvider *sound);
		bool stop(SoundChNum channel);
		bool pause(SoundChNum channel);
		bool resume(SoundChNum channel);
		SoundState_t state(SoundChNum channel);

		SoundChNum playAuto(SoundProvider sound, SoundVolume vol); // Auto select channel and play sound on it (if no aviable, count of channels will be returned)

		bool stopAll();
		bool pauseAll();
		bool resumeAll();
};

#endif
