#include <sound.h>

SoundProvider::SoundProvider() {
	queue = xQueueCreate(CONFIG_SND_PROVIDER_MAIN_QUEUE_SIZE, sizeof(SoundData));
	assert(queue != NULL);
	controlQueue = xQueueCreate(CONFIG_SND_PROVIDER_CONTROL_QUEUE_SIZE, sizeof(SoundProviderControl));
	assert(controlQueue != NULL);
}

SoundProvider::~SoundProvider() {
	vQueueDelete(queue);
	vQueueDelete(controlQueue);
}

void SoundProvider::postSample(SoundData sample) {
	xQueueSendToBack(queue, &sample, portMAX_DELAY);
}

void SoundProvider::postControl(SoundProviderControl ctrl) {
	xQueueSendToBack(controlQueue, &ctrl, portMAX_DELAY);
}

void SoundProvider::queueReset() {
	xQueueReset(queue);
}
