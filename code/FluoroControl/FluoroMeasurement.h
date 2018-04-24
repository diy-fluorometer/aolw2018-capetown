#ifndef __FLUORO_MEASUREMENT_H__
#define __FLUORO_MEASUREMENT_H__

#include "Arduino.h"
#include "FluoroDevice.h"

#define ERR_NO_SENSOR         -1
#define ERR_INVALID_PARAMETER -2
#define ERR_OVERFLOW          -3
#define ERR_NOT_CALIBRATED    -4
#define ERR_UNINITIALIZED     -5
#define ERR_OUT_OF_RANGE      -6
#define NO_ERROR               0

#define REF1  1
#define REF2  2

enum GainLevel {
  GLOW,
  GMEDIUM,
  GHIGH,
  GMAX,
  GINVALID
};

class Measurement {

   public:
    uint32_t timestamp = 0;   // Time as returned by millis() when the value was read from the sensor
    int32_t value = -23;       // The value read from the sensor
    boolean is_valid = true;
};

#endif // __FLUORO_MEASUREMENT_H__