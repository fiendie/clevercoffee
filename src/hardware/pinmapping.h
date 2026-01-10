/**
 * @file pinmapping.h
 *
 * @brief Default GPIO pin mapping
 *
 */

#pragma once

#ifdef BOARD_ESP32_S3
#define TARGET_BOARD_ESP32_S3
#elif defined(BOARD_ESP32_CLASSIC)
#define TARGET_BOARD_ESP32_CLASSIC
#else
// Default to ESP32 Classic for backward compatibility
#define TARGET_BOARD_ESP32_CLASSIC
#endif

/* available pins
https://randomnerdtutorials.com/esp32-s3-devkitc-pinout-guide/
https://github.com/atomic14/esp32-s3-pinouts?tab=readme-ov-file

20 Analog-to-Digital Converter (ADC) channels
4 SPI interfaces
3 UART interfaces
2 I2C interfaces
8 PWM output channel
2 I2S interfaces
14 Capacitive sensing GPIOs


AVOID!
GPIO 26 (Flash/PSRAM SPICS1)
GPIO 27 (Flash/PSRAM SPIHD)
GPIO 28 (Flash/PSRAM SPIWP)
GPIO 29 (Flash/PSRAM SPICS0)
GPIO 30 (Flash/PSRAM SPICLK)
GPIO 31 (Flash/PSRAM SPIQ)
GPIO 32 (Flash/PSRAM SPID)
strapping pins
GPIO 0
GPIO 3
GPIO 45
GPIO 46
octal ram
GPIO 35
GPIO 36
GPIO 37



RTCGPIOs
RTC_GPIO0  (GPIO0)
to
RTC_GPIO21 (GPIO21)

SPI	MOSI	MISO	CLK	CS
HSPI (SPI 2)	GPIO 11	GPIO 13	GPIO 12	GPIO 10
VSPI (SPI 3)	GPIO 35	GPIO 37	GPIO 36	GPIO 39



GPIOs on board (36)
0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21
35,36,37,38,39,40,41,42,43,44,45,46,47,48

minus avoid (32)
1,2,
4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21
38,39,40,41,42,43,44,
47,48

left to use
*/

#ifdef TARGET_BOARD_ESP32_S3
// ESP32-S3 Pin Mapping (16MB Flash, 8MB PSRAM)
#define PIN_POWERSWITCH 15
#define PIN_BREWSWITCH  16
#define PIN_STEAMSWITCH 17
#define PIN_WATERSWITCH 18

#define PIN_ROTARY_DT  4      // tested S2
#define PIN_ROTARY_CLK 5      // tested S1
#define PIN_ROTARY_SW  6      // tested SW

#define PIN_TEMPSENSOR      7 // tested as CS pin kType
#define PIN_TEMPSENSOR2     10
#define PIN_WATERTANKSENSOR 20
#define PIN_HXDAT           19
#define PIN_HXDAT2          44
#define PIN_HXCLK           47
#define PIN_HXCLK2          48 // could be done in software by writing a custom read function

#define PIN_VALVE   12
#define PIN_PUMP    13
#define PIN_PUMP2   41
#define PIN_HEATER  14
#define PIN_HEATER2 42

#define PIN_STATUSLED 38
#define PIN_BREWLED   39
#define PIN_STEAMLED  40

#define PIN_ZC 21

#define PIN_I2CSCL        9  // tested, HWI2C
#define PIN_I2CSDA        8  // tested, HWI2C
#define PIN_SPIMOSI       11 // tested, HWSPI
#define PIN_SPIMISO       13 // tested, HWSPI
#define PIN_SPICLK        12 // tested, HWSPI
#define PIN_SPIDC         46 // tested, strapping
#define PIN_OLEDCS        0  // tested, strapping
#define PIN_OLEDBACKLIGHT 3 // not tested
#define PIN_OLEDRESET     18 // not used, currently on Enable Pin and testing stability

#else
// ESP32 Classic Pin Mapping
#define PIN_POWERSWITCH 39
#define PIN_BREWSWITCH  3
#define PIN_STEAMSWITCH 35
#define PIN_WATERSWITCH 36

#define PIN_ROTARY_DT  4
#define PIN_ROTARY_CLK 3
#define PIN_ROTARY_SW  5

#define PIN_TEMPSENSOR      16
#define PIN_WATERTANKSENSOR 23
#define PIN_HXDAT           32
#define PIN_HXDAT2          25
#define PIN_HXCLK           33

#define PIN_VALVE  17
#define PIN_PUMP   27
#define PIN_HEATER 2

#define PIN_STATUSLED 26
#define PIN_BREWLED   19
#define PIN_STEAMLED  1

#define PIN_ZC 18

#define PIN_I2CSCL 22
#define PIN_I2CSDA 21

#endif
