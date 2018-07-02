#ifndef __FLUORO_RUN_H__
#define __FLUORO_RUN_H__

#include "FluoroMeasurement.h"

// Apparently, dynamic allocation of memory is no bueno in the Arduino world,
// so let's fix the maximum number of measurements per run.

#define MAX_MEASUREMENTS 20

class Run {
  
  public:
    uint8_t n = 0;
    String run_name = String("default");
    boolean active = false;
    Measurement m[MAX_MEASUREMENTS];
    uint8_t num_measurements = 0;
    uint8_t idx = 0;
    
    void reset_measurements() {
      for (int i = 0; i < MAX_MEASUREMENTS; i++) {
        m[i].reset();
      }
    }
    
    void start(uint8_t n, String nm) {
      if (num_measurements <= MAX_MEASUREMENTS) {
          num_measurements = n;
          run_name = nm;
          reset_measurements();
          active = true;
      }
    }

    void start() {
      num_measurements = 1;
      run_name = String("default");
      reset_measurements();
      active = true;
    }
};

#endif // __FLUORO_RUN_H__
