#ifndef __FLUORO_MEASUREMENT_H__
#define __FLUORO_MEASUREMENT_H__

#include "Arduino.h"
#include "FluoroDevice.h"
#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h"

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
    int32_t value_max = -23;       // The value read from the sensor
    int32_t value_high = -23;       // The value read from the sensor
    int32_t value_med = -23;       // The value read from the sensor
    int32_t value_low = -23;       // The value read from the sensor
    float value = -23;

    boolean is_valid = false;
    
    void reset() {
      value_max = value_high = value_med = value_low = value = -23;
      is_valid = false;
    }
    
    
  /************************************************************************/
  /*!
  Adapted from https://github.com/adafruit/Adafruit_TSL2591_Library/blob/master/Adafruit_TSL2591.cpp
      @brief  Calculates the visible Lux based on the two light sensors
      @param  ch0 Data from channel 0 (IR+Visible)
      @param  ch1 Data from channel 1 (IR)
      @returns Lux, based on AMS coefficients
  */
  /**************************************************************************/
  static float calculateLux(uint16_t ch0, uint16_t ch1, tsl2591IntegrationTime_t _integration, tsl2591Gain_t _gain)
  {
    float    atime, again;
    float    cpl, lux1, lux2, lux;
    uint32_t chan0, chan1;
  
    // Check for overflow conditions first
    if ((ch0 == 0xFFFF) | (ch1 == 0xFFFF))
    {
      // Signal an overflow
      return 0;
    }
  
    // Note: This algorithm is based on preliminary coefficients
    // provided by AMS and may need to be updated in the future
  
    switch (_integration)
    {
      case TSL2591_INTEGRATIONTIME_100MS :
        atime = 100.0F;
        break;
      case TSL2591_INTEGRATIONTIME_200MS :
        atime = 200.0F;
        break;
      case TSL2591_INTEGRATIONTIME_300MS :
        atime = 300.0F;
        break;
      case TSL2591_INTEGRATIONTIME_400MS :
        atime = 400.0F;
        break;
      case TSL2591_INTEGRATIONTIME_500MS :
        atime = 500.0F;
        break;
      case TSL2591_INTEGRATIONTIME_600MS :
        atime = 600.0F;
        break;
      default: // 100ms
        atime = 100.0F;
        break;
    }
  
    switch (_gain)
    {
      case TSL2591_GAIN_LOW :
        again = 1.0F;
        break;
      case TSL2591_GAIN_MED :
        again = 25.0F;
        break;
      case TSL2591_GAIN_HIGH :
        again = 428.0F;
        break;
      case TSL2591_GAIN_MAX :
        again = 9876.0F;
        break;
      default:
        again = 1.0F;
        break;
    }
  
    // cpl = (ATIME * AGAIN) / DF
    cpl = (atime * again) / TSL2591_LUX_DF;
  
    // Original lux calculation (for reference sake)
    //lux1 = ( (float)ch0 - (TSL2591_LUX_COEFB * (float)ch1) ) / cpl;
    //lux2 = ( ( TSL2591_LUX_COEFC * (float)ch0 ) - ( TSL2591_LUX_COEFD * (float)ch1 ) ) / cpl;
    //lux = lux1 > lux2 ? lux1 : lux2;
  
    // Alternate lux calculation 1
    // See: https://github.com/adafruit/Adafruit_TSL2591_Library/issues/14
    lux = ( ((float)ch0 - (float)ch1 )) * (1.0F - ((float)ch1/(float)ch0) ) / cpl;
  
    // Alternate lux calculation 2
    //lux = ( (float)ch0 - ( 1.7F * (float)ch1 ) ) / cpl;
  
    // Signal I2C had no errors
    return lux;
  }
};

#endif // __FLUORO_MEASUREMENT_H__
