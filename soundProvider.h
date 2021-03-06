#pragma once

class SoundMixer;

class SoundProvider { // Abstract interface for sound providers. Include queues initialisation/deinitialisation.
	protected:
		QueueHandle_t queue = NULL; // Read from here
		QueueHandle_t controlQueue = NULL; // Read controlling data from here

		// PROVIDER CONTROL INTERFACE START
		virtual void provider_start() = 0; // Start filling (should be ok if started)
		virtual void provider_pause() {}; // Optional methods for extra optimisation
		virtual void provider_resume() {}; 
		virtual void provider_stop() = 0; // Stop filling (should be ok if isn't started)

		virtual void provider_restart() {provider_stop(); provider_start();} // This one calls if track repeats (default implementation should work, but it is better to write your own)
		// PROVIDER CONTROL INTERFACE END

		void postSample(SoundData sample);
		void postControl(SoundProviderControl ctrl);

		void queueReset();

		virtual unsigned long int getFrequency() = 0; // Frequency in Hz, should be constant

	private:
		unsigned int divisor = 1; // For different frequencies in same mixer
		SoundData actual = 0;

	public:
		SoundProvider();
		virtual ~SoundProvider();

		bool repeat = false; // Implementation souldn't use any optimisations based on this

	friend SoundMixer;
};
