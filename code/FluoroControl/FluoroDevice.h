#ifndef __FLUORO_DEVICE_H__
#define __FLUORO_DEVICE_H__

#include "Arduino.h"

#define BLUE_LED_PIN D3


// TODO work with defines to change the pin names depending on whether it's
// a NodeMCU or an Arduino board
class FluoroDevice {
  public:
    uint8_t pin_blue_led = D3;
    uint8_t pin_sensor_CLK = D1;
    uint8_t pin_sensor_SDA = D2;
};

static FluoroDevice device;

#endif // __FLUORO_DEVICE_H__