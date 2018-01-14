#include <soundProviderTask.h>

SoundProviderTask::SoundProviderTask() {}
SoundProviderTask::~SoundProviderTask() {}

void SoundProviderTask::provider_start() {
	if (taskHandle == NULL) {
		task_start();
		xTaskCreate(reinterpret_cast<TaskFunction_t>(&SoundProviderTask::taskProviderCode), "SProvTask", stackSize, this, 10, &taskHandle);
	}
}

void SoundProviderTask::provider_stop() {
	if (taskHandle != NULL) {
		vTaskDelete(taskHandle);
		taskHandle = NULL;
		task_stop();
	}
}

void SoundProviderTask::stopFromTask() {
	TaskHandle_t handle = taskHandle;
	taskHandle = NULL;
	vTaskDelete(handle);
	while(true) {};
}

void SoundProviderTask::taskProviderCode() {
	task_code();
	postControl(END);
	stopFromTask();
}
