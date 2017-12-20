#ifndef SOUND_H
#define SOUND_H
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

#include <soundMixer.h>
#include <soundProvider.h>
#endif
