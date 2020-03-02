#include "globalPlatform.h"


#ifdef ARDUINO_ARCH_SAMD
SamdPlatform platform;
#elif ARDUINO_ARCH_ESP8266
EspPlatform platform;
#elif ARDUINO_ARCH_ESP32
#define LED_BUILTIN 13
Esp32Platform platform;
#else
LinuxPlatform platform;
#define LED_BUILTIN 0
#endif
