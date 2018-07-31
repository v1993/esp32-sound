#pragma once

#include <stdint.h>
#include <iostream>
#include <numeric>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <freertos/queue.h>

#define SOUND_FREQ_TO_DELAY(f) (1000000/f)

class SoundProvider;

typedef unsigned int SoundChNum;
typedef uint8_t SoundData;
typedef long unsigned int SoundPos;
typedef unsigned int SoundVolume;

enum SoundState {
	STOPPED,
	PLAYING,
	PAUSED
};

enum SoundProviderControl {
	END,
	FAILURE
};

enum SoundEvent {
	STOP,
	START,
	PAUSE,
	RESUME,
	VOLSET
};

struct SoundControl {
	SoundEvent event;
	SoundChNum channel;
	SoundProvider *provider;
	SoundVolume vol;
};

#include <soundMixer.h>
#include <soundProvider.h>
