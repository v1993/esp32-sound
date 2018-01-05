#include <soundProvider/pcm.h>

SoundProviderPcm::SoundProviderPcm(SoundData *dataarg, SoundPos lenarg, unsigned long int freqarg) {
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

void SoundProviderPcm::provider_start() {
	if (taskHandle == NULL) {
		xTaskCreate(reinterpret_cast<TaskFunction_t>(&SoundProviderPcm::taskCode), "SoundProviderPcm", 256, this, 10, &taskHandle);
	}
}

void SoundProviderPcm::provider_stop() {
	if (taskHandle != NULL) {
		vTaskDelete(taskHandle);
		taskHandle = NULL;
	}
}

void SoundProviderPcm::taskCode() {
	for (SoundPos i = 0; i < len; i++) {
		postSample(data[i]);
	}
	postControl(END);
	vTaskDelete(taskHandle);
}
