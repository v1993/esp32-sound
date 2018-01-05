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

		void incSound() volatile; // Increment counter
		void decSound() volatile; // Decrement counter

		void soundCallback() volatile; // Play one step
		bool handleQueue() volatile; // Handle suspended events (SAFE)
		void setupTimer() volatile; // Set divisors and start timer

		void addEvent(SoundControl_t event) volatile;

		void checkTimer() volatile; // Start one-shot "promo-"timer if isn't active
	public:

		SoundMixer(SoundChNum normal_channels, SoundChNum auto_channels, dac_channel_t dac); // Setup SoundMixer
		~SoundMixer(); // Destroy extra stuff

		void play(SoundChNum channel, SoundProvider *sound) volatile;
		void stop(SoundChNum channel) volatile;
		void pause(SoundChNum channel) volatile;
		void resume(SoundChNum channel) volatile;
		void setVolume(SoundChNum channel, SoundVolume vol) volatile;
		void getVolume(SoundChNum channel) volatile;
		SoundState_t state(SoundChNum channel) volatile; // SAFE

		SoundChNum playAuto(SoundProvider *sound, SoundVolume vol) volatile; // Auto select channel and play sound on it (if no aviable, count of channels will be returned)

		void stopAll() volatile;
		void pauseAll() volatile;
		void resumeAll() volatile;
};

#endif
