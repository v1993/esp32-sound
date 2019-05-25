#include <soundProviderTask.h>

extern "C" void sound_provider_start(void* arg) {
	static_cast<Sound::SoundProviderTask*>(arg)->taskProviderCode();
}

namespace Sound {
	SoundProviderTask::SoundProviderTask() {
	}
	SoundProviderTask::~SoundProviderTask() {
	}

	void SoundProviderTask::unconditionalStart() {
		xTaskCreate(sound_provider_start, "SProvTask", stackSize, this, 10, &taskHandle);
	}

	void SoundProviderTask::provider_start() {
		if (taskHandle == nullptr) {
			task_prestart();
			unconditionalStart();
			task_poststart();
		}
	}

	void SoundProviderTask::provider_stop() {
		if (taskHandle != nullptr) {
			task_prestop();
			vTaskDelete(taskHandle);
			taskHandle = nullptr;
			task_poststop();
			queueReset();
		}
	}

	void SoundProviderTask::provider_restart() {
		if (taskHandle != nullptr) {
			task_prestop();
			vTaskDelete(taskHandle);
			task_poststop();
		}
		unconditionalStart();
	}

	void SoundProviderTask::stopFromTask() {
		TaskHandle_t handle = taskHandle;
		taskHandle = nullptr;
		vTaskDelete(handle);
		while(true) {
		};
	}

	void SoundProviderTask::taskProviderCode() {
		task_code();
		stopFromTask();
	}
}
