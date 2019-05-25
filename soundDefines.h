#pragma once

#include <stdint.h>
#include <numeric>
#include <memory>

extern "C" {
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <freertos/queue.h>
#include <esp_timer.h>
#include <driver/gpio.h>
#include <driver/dac.h>
#include <esp_attr.h>
}

#define USING_NS_SOUND using namespace Sound

namespace Sound {
	class SoundProvider;
	class SoundMixer;

	using SoundChNum = unsigned int;
	using SoundData = uint8_t;
	using SoundPos = long unsigned int;
	using SoundVolume = unsigned int;

	enum SoundState {
		STOPPED,
		PLAYING,
		PAUSED
	};

	enum SoundProviderControl {
		FREQUENCY_UPDATE,
		END,
		FAILURE
	};

	enum SoundEvent {
		STOP,
		START,
		RESTART,
		PAUSE,
		RESUME,
		VOLSET
	};

	struct SoundControl {
			SoundEvent event;
			SoundChNum channel;
			std::shared_ptr<SoundProvider> provider;
			SoundVolume vol;
	};
}

