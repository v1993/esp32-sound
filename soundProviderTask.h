#ifndef SOUND_PROVIDER_TASK_H
#define SOUND_PROVIDER_TASK_H
#include <sound.h>

class SoundProviderTask: public SoundProvider {
	protected:
		// TASK PROVIDER INTERFACE START
		virtual void task_start() {};
		virtual void task_code() = 0;
		virtual void task_stop() {};
		// TASK PROVIDER INTERFACE END

		// PROVIDER CONTROL INTERFACE START
		void provider_start(); // Start filling (should be ok if started)
		void provider_stop(); // Stop filling (should be ok if isn't started)
		// PROVIDER CONTROL INTERFACE END

		void taskProviderCode();
		TaskHandle_t taskHandle = NULL;
	public:
		SoundProviderTask();
		virtual ~SoundProviderTask();
};

#endif
