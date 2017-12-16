# esp32-sound
C++ component for esp-idf which allow playing sound via DAC with mixing. It require raw 8-bit sound (waw data in `uint8_t` array) with custom frequency.

This module is not finished, so use it at your own risk. API may change without notice.

# IMPORTANT

Due to bug in [esp-idf](https://github.com/espressif/esp-idf/issues/1391) sound may suddenly stop. Workaround is using as low as possible frequency (it require more time to bug on 8000Hz than on 16000Hz)
