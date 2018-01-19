#ifndef SOUND_PROVIDER_TASK_H
#define SOUND_PROVIDER_TASK_H
#include <sound.h>

class SoundProviderTask: public SoundProvider {
	protected:
		// TASK PROVIDER INTERFACE START
		virtual void task_prestart() {};
		virtual void task_poststart() {};
		virtual void task_code() = 0;
		virtual void task_prestop() {};
		virtual void task_poststop() {};
		// TASK PROVIDER INTERFACE END

		// PROVIDER CONTROL INTERFACE START
		void provider_start(); // Start filling (should be ok if started)
		void provider_stop(); // Stop filling (should be ok if isn't started)
		// PROVIDER CONTROL INTERFACE END

		void taskProviderCode();
		void stopFromTask();
		TaskHandle_t taskHandle = NULL;

		size_t stackSize = 2048;
	public:
		SoundProviderTask();
		virtual ~SoundProviderTask();
};

#endif
