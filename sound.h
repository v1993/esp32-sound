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

typedef enum SoundState {
	STOPPED,
	PLAYING,
	PAUSED
} SoundState_t;

typedef enum SoundProviderControl {
	END,
	FAILURE
} SoundProviderControl_t;

typedef enum SoundEvent {
	STOP,
	START,
	PAUSE,
	RESUME,
	VOLSET
} SoundEvent_t;

typedef struct SoundControl {
	SoundEvent_t event;
	SoundChNum channel;
	SoundProvider *provider;
	SoundVolume vol;
} SoundControl_t;

#include <soundMixer.h>
#include <soundProvider.h>
