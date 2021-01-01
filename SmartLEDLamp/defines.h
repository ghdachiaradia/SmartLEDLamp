#ifndef DEFINES_H_
#define DEFINES_H_

// Comment next line to disable IR code if you don't use an IR receiver
//#define IR_ENABLE

#define UDP_PORT 6789

#define LEDS_WIDTH 4
#define LEDS_HEIGHT 40

#ifdef IR_ENABLE
#define PIN_RECV_IR D3
#endif

#define PIN_CLOCK D2
#define PIN_DATA D1

#define STRIP_RGB_ORDER RGB
#define STRIP_CHIPSET WS2801

#define BTN_BRIGHTER 1
#define BTN_DARKER 2
#define BTN_PAUSE 3
#define BTN_POWER 4
#define BTN_R 5
#define BTN_G 6
#define BTN_B 7
#define BTN_W 8
#define BTN_RED_UP 25
#define BTN_GREEN_UP 26
#define BTN_BLUE_UP 27
#define BTN_QUICK 28
#define BTN_SHIFT_UP 29

#define BTN_RED_DOWN 35
#define BTN_GREEN_DOWN 36
#define BTN_BLUE_DOWN 37
#define BTN_SLOW 38
#define BTN_SHIFT_DOWN 39


#define BTN_DIY1 55
#define BTN_DIY2 56
#define BTN_DIY3 57
#define BTN_AUTO 58
#define BTN_DIY4 59
#define BTN_DIY5 60
#define BTN_DIY6 61
#define BTN_FLASH 62
#define BTN_JUMP3 63
#define BTN_JUMP7 64
#define BTN_FADE3 65
#define BTN_FADE7 66
#define BTN_SOLID 67

#endif /* DEFINES_H_ */
