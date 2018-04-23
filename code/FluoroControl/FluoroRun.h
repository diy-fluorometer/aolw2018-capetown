#ifndef __FLUORO_RUN_H__
#define __FLUORO_RUN_H__

#include "FluoroMeasurement.h"

// Apparently, dynamic allocation of memory is no bueno in the Arduino world,
// so let's fix the maximum number of measurements per run.

#define MAX_MEASUREMENTS 100

class Run {
  
  public:
    uint8_t n = 0;
    String run_name = String("default");
    boolean active = false;
    Measurement m[MAX_MEASUREMENTS];
    uint8_t idx = 0;
};

#endif // __FLUORO_RUN_H__