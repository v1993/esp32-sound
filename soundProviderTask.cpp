#include <soundProviderTask.h>
#include <esp_task_wdt.h>

SoundProviderTask::SoundProviderTask() {}
SoundProviderTask::~SoundProviderTask() {}

void SoundProviderTask::provider_start() {
	if (taskHandle == NULL) {
		task_prestart();
		xTaskCreate(reinterpret_cast<TaskFunction_t>(&SoundProviderTask::taskProviderCode), "SProvTask", stackSize, this, 10, &taskHandle);
		task_poststart();
	}
}

void SoundProviderTask::provider_stop() {
	if (taskHandle != NULL) {
		task_prestop();
		vTaskDelete(taskHandle);
		taskHandle = NULL;
		task_poststop();
		queueReset();
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
