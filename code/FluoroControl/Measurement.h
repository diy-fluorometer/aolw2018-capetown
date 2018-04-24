#ifndef __MEASUREMENT_H__
#define __MEASUREMENT_H__

#include "Arduino.h"

#define ERR_NO_SENSOR         -1
#define ERR_INVALID_PARAMETER -2
#define ERR_OVERFLOW          -3
#define ERR_NOT_CALIBRATED    -4
#define ERR_UNINITIALIZED     -5
#define ERR_OUT_OF_RANGE      -6
#define NO_ERROR               0

enum GainLevel {
  GLOW,
  GMEDIUM,
  GHIGH,
  GMAX,
  GINVALID
};

class Measurement {

   public:
    String run_name;      // The name of the run this measurement belongs to
    uint32_t timestamp;   // Time as returned by millis() when the value was read from the sensor
    uint32_t seq_id;      // This is the seq_id'th measurement in this run TODO overflow?
    int32_t value;        // The value read from the sensor
    GainLevel gain_level; // Which of the four gain levels the value was read at.
    boolean is_valid;
    int32_t error;

    Measurement() {
      run_name = "";
      timestamp = 0;
      seq_id = 0;
      value = -1;
      gain_level = GINVALID;
      is_valid = false;
      error = ERR_UNINITIALIZED;
    }
  
    String to_logstring() {
      // The returned string will include all the fields except for is_valid and error
      
      String str = String(value) + "\t" +
        String(timestamp) + "\t" +
        String(run_name) + "\t" +
        String(seq_id) + "\t";
      switch (gain_level) {
        case GLOW: str += "LOW"; break;
        case GMEDIUM: str += "MEDIUM"; break;
        case GHIGH: str += "HIGH"; break;
        case GMAX: str += "MAX"; break;
        case GINVALID: // fall through
        default:
          str += "INVALID";
      }
      str += "\n";
      return str;
    }

    void set(String run_name, uint32_t timestamp, uint32_t seq_id, int32_t value, GainLevel gain_level) {
      this->run_name = run_name;
      this->timestamp = timestamp;
      this->seq_id = seq_id;
      this->value = value;
      this->gain_level = gain_level;
      is_valid = true;
      error = NO_ERROR;
    }

    void set_error(int32_t error_code) {
      error = error_code;
      is_valid = false;
    }
};

class Run {
  
  public:
    String name;
    uint32_t last_id;

   	void to_default() {
   		name = "default";
   		last_id = 0;
   	}
};

#endif // __MEASUREMENT_H__