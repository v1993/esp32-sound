#pragma once
#include "soundDefines.h"
#include <array>
#include <queue>
#include <list>
#include <mutex>

namespace Sound {
	class SoundMixer {
		protected:
			esp_timer_handle_t timer = NULL; // Timer for this instance
			SemaphoreHandle_t mutex = NULL; // Mutex for this instance
			SemaphoreHandle_t timerMutex = NULL; // Mutex for timer control

			std::queue<SoundControl, std::list<SoundControl>> queue; // Queue for this instance, not FreeRTOS due to smart pointers
			// Refer to https://stackoverflow.com/q/51632219/5697743
			std::mutex queueMutex; // Mutex for queue

			dac_channel_t dacCh; // DAC channel for sound

			unsigned long int counter = 1; // Only for callback
			unsigned long int counterMax = 1;

			SoundChNum chCount; // Total channels count, <= CONFIG_SND_MAX_CHANNELS
			SoundChNum chFirstAuto; // Number of first "auto" channel
			SemaphoreHandle_t chActiveCount = NULL; // Count of active channels (to control timer)
			std::array<std::shared_ptr<SoundProvider>, CONFIG_SND_MAX_CHANNELS> chSound; // Sound provider pointers
			bool chActive[CONFIG_SND_MAX_CHANNELS]; // Active channels, UNSAFE
			bool chPaused[CONFIG_SND_MAX_CHANNELS]; // Paused channels, UNSAFE

			SoundVolume chVolume[CONFIG_SND_MAX_CHANNELS]; // Volume map

			void incSound(); // Increment counter
			void decSound(); // Decrement counter
	
			void soundCallback(); // Play one step
			bool handleQueue(); // Handle suspended events (SAFE)
			void setupTimer(); // Set divisors and start timer

			void addEvent(const SoundControl& event);

			void checkTimer(); // Start one-shot "promo-"timer if isn't active
		public:
			SoundMixer(SoundChNum normal_channels, SoundChNum auto_channels, dac_channel_t dac); // Setup SoundMixer
			~SoundMixer(); // Destroy extra stuff

			void play(SoundChNum channel, const std::shared_ptr<SoundProvider>& sound);
			void stop(SoundChNum channel);
			void pause(SoundChNum channel);
			void resume(SoundChNum channel);
			void setVolume(SoundChNum channel, SoundVolume vol);
			SoundVolume getVolume(SoundChNum channel);
			SoundState state(SoundChNum channel); // SAFE

			SoundChNum playAuto(const std::shared_ptr<SoundProvider>& sound, SoundVolume vol); // Auto select channel and play sound on it (if no aviable, count of channels will be returned)

			void stopAll();
			void pauseAll();
			void resumeAll();
	};
}
