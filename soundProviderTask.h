#pragma once
#include <sound.h>

namespace Sound {
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
			virtual void provider_start() override final; // Start filling (should be ok if started)
			virtual void provider_stop() override final; // Stop filling (should be ok if isn't started)
			virtual void provider_restart() override; // This one calls if track repeats (default implementation MAY NOT work)
			// PROVIDER CONTROL INTERFACE END

			void taskProviderCode();
			void stopFromTask();
			void unconditionalStart();
			TaskHandle_t taskHandle = nullptr;

			size_t stackSize = 2048;
		public:
			SoundProviderTask();
			virtual ~SoundProviderTask();
	};
}
