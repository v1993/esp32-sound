#ifndef SOUND_PROVIDER_PCM_H
#define SOUND_PROVIDER_PCM_H
#include <sound.h>

class SoundProviderPcm: public SoundProvider {
	protected:
		// PROVIDER CONTROL INTERFACE START
		void provider_start(); // Start filling (should be ok if started)
		void provider_stop(); // Stop filling (should be ok if isn't started)
		// PROVIDER CONTROL INTERFACE END
		unsigned long int getFrequency();

		void taskCode();
		TaskHandle_t taskHandle = NULL;

		unsigned long int freq = 0;
		SoundData *data = NULL;
		SoundPos len = 0;
	public:
		SoundProviderPcm(SoundData *dataarg, unsigned long int freqarg, SoundPos lenarg);
		virtual ~SoundProviderPcm();
}
#endif
