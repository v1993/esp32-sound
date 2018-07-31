#pragma once

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
		SemaphoreHandle_t mutex = NULL; // Mutex for this instance
		SemaphoreHandle_t timerMutex = NULL; // Mutex for timer control
		QueueHandle_t queue = NULL; // Queue for this instance
		dac_channel_t dacCh; // DAC channel for sound

		unsigned long int counter = 1; // Only for callback
		unsigned long int counterMax = 1;

		SoundChNum chCount; // Total channels count, <= CONFIG_SND_MAX_CHANNELS
		SoundChNum chFirstAuto; // Number of first "auto" channel
		SemaphoreHandle_t chActiveCount = NULL; // Count of active channels (to control timer)
		SoundProvider* chSound[CONFIG_SND_MAX_CHANNELS]; // Sound provider pointers (you should carry about memory by youself)
		bool chActive[CONFIG_SND_MAX_CHANNELS]; // Active channels, UNSAFE
		bool chPaused[CONFIG_SND_MAX_CHANNELS]; // Paused channels, UNSAFE

		SoundVolume chVolume[CONFIG_SND_MAX_CHANNELS]; // Volume map

		void incSound(); // Increment counter
		void decSound(); // Decrement counter

		void soundCallback(); // Play one step
		bool handleQueue(); // Handle suspended events (SAFE)
		void setupTimer(); // Set divisors and start timer

		void addEvent(SoundControl event);

		void checkTimer(); // Start one-shot "promo-"timer if isn't active
	public:

		SoundMixer(SoundChNum normal_channels, SoundChNum auto_channels, dac_channel_t dac); // Setup SoundMixer
		~SoundMixer(); // Destroy extra stuff

		void play(SoundChNum channel, SoundProvider *sound);
		void stop(SoundChNum channel);
		void pause(SoundChNum channel);
		void resume(SoundChNum channel);
		void setVolume(SoundChNum channel, SoundVolume vol);
		SoundVolume getVolume(SoundChNum channel);
		SoundState state(SoundChNum channel); // SAFE

		SoundChNum playAuto(SoundProvider *sound, SoundVolume vol); // Auto select channel and play sound on it (if no aviable, count of channels will be returned)

		void stopAll();
		void pauseAll();
		void resumeAll();
};
