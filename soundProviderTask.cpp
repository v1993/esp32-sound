#include <soundProviderTask.h>

SoundProviderTask::SoundProviderTask() {}
SoundProviderTask::~SoundProviderTask() {}

void SoundProviderPcm::provider_start() {
	if (taskHandle == NULL) {
		task_start();
		xTaskCreate(reinterpret_cast<TaskFunction_t>(&SoundProviderTask::taskProviderCode), "SProvTask", 512, this, 10, &taskHandle);
	}
}

void SoundProviderTask::provider_stop() {
	if (taskHandle != NULL) {
		task_stop();
		vTaskDelete(taskHandle);
		taskHandle = NULL;
	}
}

void SoundProviderTask::taskProviderCode() {
	task_code();
	postControl(END);
	TaskHandle_t handle = taskHandle;
	taskHandle = NULL;
	vTaskDelete(handle);
}
