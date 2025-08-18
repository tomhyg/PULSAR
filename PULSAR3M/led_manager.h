#ifndef LED_MANAGER_H
#define LED_MANAGER_H

#include "config.h"

class LEDManager {
public:
    static void init();
    static void test();
    static void setState(LEDState state);
    static void update();
    static void setColor(uint8_t r, uint8_t g, uint8_t b);
    
private:
    static Adafruit_NeoPixel strip;
    static LEDState currentState;
    static unsigned long lastUpdate;
    static bool lastLedState;
};

#endif
