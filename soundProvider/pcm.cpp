#include <soundProvider/pcm.h>

SoundProviderPcm::SoundProviderPcm(const SoundData *dataarg, SoundPos lenarg, unsigned long int freqarg) {
	data = dataarg;
	freq = freqarg;
	len = lenarg;
}

SoundProviderPcm::~SoundProviderPcm() {
	provider_stop();
}

unsigned long int SoundProviderPcm::getFrequency() {
	return freq;
}

void SoundProviderPcm::task_code() {
	for (SoundPos i = 0; i < len; i++) {
		postSample(data[i]);
	}
}
