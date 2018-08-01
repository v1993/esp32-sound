#include <sound.h>

namespace Sound {
	SoundProvider::SoundProvider() {
		queue = xQueueCreate(CONFIG_SND_PROVIDER_MAIN_QUEUE_SIZE, sizeof(SoundData));
		assert(queue != nullptr);
		controlQueue = xQueueCreate(CONFIG_SND_PROVIDER_CONTROL_QUEUE_SIZE, sizeof(SoundProviderControl));
		assert(controlQueue != nullptr);
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
}
