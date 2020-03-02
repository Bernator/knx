#pragma once
#ifndef KNX_STACK_SRC_GLOBALPLATFORM_H_
#define KNX_STACK_SRC_GLOBALPLATFORM_H_


#ifdef ARDUINO_ARCH_SAMD
#include "samd_platform.h"
extern SamdPlatform platform;
#elif ARDUINO_ARCH_ESP8266
#include "esp_platform.h"
#elif ARDUINO_ARCH_ESP32
#define LED_BUILTIN 13
#include "esp32_platform.h"
#else
#include "linux_platform.h"
#define LED_BUILTIN 0
#endif


#endif /* KNX_STACK_SRC_GLOBALPLATFORM_H_ */
