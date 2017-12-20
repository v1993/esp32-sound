#ifndef SOUND_PROVIDER_H
#define SOUND_PROVIDER_H
class SoundMixer;

class SoundProvider { // Abstract interface for sound providers
	protected:
		// BUFFER INTERFACE START
		SoundData* buf; // Read from here
		SoundPos buf_len = 0; // Buffer length (0 mean nothing to read), valid only after buf_init
		SoundPos buf_pos = 0; // Position (in buffer)

		virtual void buf_init() = 0; // Init buffer (reset position and fill with first piece)
		virtual void buf_update() = 0; // Update buffer with next piece of data
		virtual void buf_deinit() {}; // Free buffer memory (optional)

		virtual void buf_reload() {buf_deinit(); buf_init();} // This one calls if track repeats (default implementation should work, but it is better to write your own)
		// BUFFER INTERFACE END

		virtual void getFrequency(); // Frequency in Hz, should be constant
	public:
		SoundProvider() {};
		virtual ~SoundProvider() {};

		bool repeat; // Implementation sholdn't carry about it

	friend SoundMixer;
};

#endif
