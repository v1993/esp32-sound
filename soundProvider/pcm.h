#ifndef SOUND_PROVIDER_PCM_H
#define SOUND_PROVIDER_PCM_H
#include <sound.h>
#include <soundProviderTask.h>

class SoundProviderPcm: public SoundProviderTask {
	protected:
		unsigned long int getFrequency();
		void task_code();

		unsigned long int freq = 0;
		const SoundData *data = NULL;
		SoundPos len = 0;
	public:
		SoundProviderPcm(const SoundData *dataarg, SoundPos lenarg, unsigned long int freqarg);
		virtual ~SoundProviderPcm();
};

#endif
