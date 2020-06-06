/* TSL2591 Digital Light Sensor */
/* Dynamic Range: 600M:1 */
/* Maximum Lux: 88K */

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h"

#include "FluoroMeasurement.h"
#include "FluoroRun.h"
#include "FluoroDevice.h"

// When using Arduino Uno:
// connect SCL to analog 5
// connect SDA to analog 4
// connect Vin to 3.3-5V DC
// connect GROUND to common ground

// When using NodeMCU Amica V2 board:
// connect SCL to D1
// connect SDA to D2
// Vin to 3.3-5V
// GND to GND

#define DEBUG 1
#define BAIL_ON_ERROR(x) { if (x == ERR_NO_SENSOR || x == ERR_INVALID_PARAMETER) { return MSRMT_ERROR; } }
#define MAX_MEASUREMENTS 500 // sanity control, to prevent me from shooting myself in the foot

// This code supports four modes for reading from the sensor:
// - auto gain / fixed time
// - auto time / fixed gain
// - auto gain / auto time
// - fixed gain / fixed time
// 
// The mode is set via commands over the serial interface.
//
// The serial commands are as follows:
//
// gain <auto|max|hi|med|lo> -- configures the gain
// time <auto|100|...|600>   -- configures the integration time
// led <on|off|auto>         -- controls the blue LED. auto: the
//                                          read command will switch on the LED
//                                          before reading the value series and
//                                          switch it off after
// read <n>                  -- takes n measurements, for n < MAX_MEASUREMENTS
//
// Note: Manually controlling the light may not appear to make much sense, but I
// wanted it during R&D to investigate and measure possible warming effects as well
// as photobleaching.
// 
// the output format is one measurement per line:
//   <i>,<value>,<gain>,<time>

Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);
bool sensor_found = false;

Run current_run;

void configure_sensor(void);
int32_t raw_read();
void calibrate(uint8_t ref);
float read_auto_gain();
float read_auto_time();
float read_auto_gain_time();

bool isAutoGain = false;
bool isAutoTime = false;
bool isAutoLed  = true;

uint32_t num_measurements = 0;

void setup(void) 
{
  Serial.begin(9600);

  if (tsl.begin()) {
    Serial.println("Found a TSL2591 sensor");
    sensor_found = true;

    // set the initial gain and integration time to default values
    tsl.setGain(TSL2591_GAIN_MAX);
    tsl.setTiming(TSL2591_INTEGRATIONTIME_600MS);
    
    pinMode(BLUE_LED_PIN, OUTPUT);
    digitalWrite(BLUE_LED_PIN, LOW);
  } 
  else {
    Serial.println("No sensor found ... check your wiring?");
  }
}

bool led_status = 0;

#define TSL2591_GMAX_FACTOR  9876
#define TSL2591_GHIGH_FACTOR  428
#define TSL2591_GMED_FACTOR    25
#define TSL2591_GLOW_FACTOR     1

void print_measurement(uint32_t i, float value) {
  Serial.print(i); Serial.print(","); Serial.print(value); Serial.print(",");
  switch (tsl.getGain()) {
    case TSL2591_GAIN_LOW:
      Serial.print("lo");
      break;
    case TSL2591_GAIN_MED:
      Serial.print("med");
      break;
    case TSL2591_GAIN_HIGH:
      Serial.print("hi");
      break;
    case TSL2591_GAIN_MAX:
      Serial.print("max");
      break;
    default:
      Serial.print("na");
  }
  Serial.print(",");
  switch (tsl.getTiming()) {
    case TSL2591_INTEGRATIONTIME_100MS:
      Serial.print("100");
      break;
    case TSL2591_INTEGRATIONTIME_200MS:
      Serial.print("200");
      break;
    case TSL2591_INTEGRATIONTIME_300MS:
      Serial.print("300");
      break;
    case TSL2591_INTEGRATIONTIME_400MS:
      Serial.print("400");
      break;
    case TSL2591_INTEGRATIONTIME_500MS:
      Serial.print("500");
      break;
    case TSL2591_INTEGRATIONTIME_600MS:
      Serial.print("600");
      break;
    default:
      Serial.print("na");
  }
  Serial.println();
}

void led_on() {
  if (isAutoLed) {
    digitalWrite(BLUE_LED_PIN, HIGH);
  } else {
    // do nothing - light control is on manual
  }
}

void led_off() {
  if (isAutoLed) {
    digitalWrite(BLUE_LED_PIN, LOW);
  } else {
    // do nothing - light control is on manual
  }
}

#define MSRMT_ERROR -1.0

void take_measurements(uint32_t num_measurements) {

  // Simplest case: Fixed integration time and fixed gain.
  if (!isAutoTime && !isAutoGain) {
    led_on();
    for (uint32_t i = 1; i <= num_measurements; i++) {
      int32_t v = raw_read();
      float value = Measurement::calculateLux(v, 0, tsl.getTiming(), tsl.getGain());
      print_measurement(i, value);
    }
    led_off();
    return;
  }

  // Fixed integration time, auto gain

  if (isAutoGain && !isAutoTime) {
    led_on();
    for (uint32_t i = 1; i <= num_measurements; i++) {
      float value = read_auto_gain();
      print_measurement(i, value);
    }
    led_off();
    return;
  }

  // Fixed gain, auto integration time

  if (!isAutoGain && isAutoTime) {
    led_on();
    for (uint32_t i = 1; i <= num_measurements; i++) {
      float value = read_auto_time();
      print_measurement(i, value);
    }
    led_off();
    return;
  }

  // Auto gain, auto integration time

  led_on();
  for (uint32_t i = 1; i <= num_measurements; i++) {
    float value = read_auto_gain_time();
    print_measurement(i, value);
  }
  led_off(); 
} 

float read_auto_gain() {

  tsl.setGain(TSL2591_GAIN_MAX);
  int32_t v = raw_read();
  if (v < 0) {
    if (v == ERR_NO_SENSOR ||
        v == ERR_INVALID_PARAMETER) {
      return MSRMT_ERROR;        
    } else {
#if DEBUG
      Serial.println("Falling through from MAX to HIGH");
#endif
      // Overflow. Fall through and try again with a different gain setting.
    }
  } else {
    // Good, we got a useable value.
#if DEBUG
    Serial.print("MAX: "); Serial.println(v);
#endif
    return Measurement::calculateLux(v, 0, tsl.getTiming(), tsl.getGain());
  }


  tsl.setGain(TSL2591_GAIN_HIGH);
  v = raw_read();
  if (v < 0) {
    if (v == ERR_NO_SENSOR ||
        v == ERR_INVALID_PARAMETER) {
      return MSRMT_ERROR;        
    } else {
#if DEBUG
      Serial.println("Falling through from HIGH to MED");
#endif
      // Overflow. Fall through and try again with a different gain setting.
    }
  } else {
    // Good, we got a useable value.
    // Adjust it for gain factor so we end up with a single scale for all values.
#if DEBUG
    Serial.print("HIGH: "); Serial.println(v);
#endif

    return Measurement::calculateLux(v, 0, tsl.getTiming(), tsl.getGain());
  }

  tsl.setGain(TSL2591_GAIN_MED);
  v = raw_read();
  if (v < 0) {
    if (v == ERR_NO_SENSOR ||
        v == ERR_INVALID_PARAMETER) {
      return MSRMT_ERROR;        
    } else {
#if DEBUG
      Serial.println("Falling through from MED to LOW");        
#endif
      // Overflow. Fall through and try again with a different gain setting.
    }
  } else {
    // Good, we got a useable value.
    // Adjust it for gain factor so we end up with a single scale for all values.
#if DEBUG
    Serial.print("MED: "); Serial.println(v);
#endif

    return Measurement::calculateLux(v, 0, tsl.getTiming(), tsl.getGain());
  }

  tsl.setGain(TSL2591_GAIN_LOW);
  v = raw_read();
  if (v < 0) {
    if (v == ERR_NO_SENSOR ||
        v == ERR_INVALID_PARAMETER) {
      return MSRMT_ERROR;        
    } else {
#if DEBUG
      Serial.println("Already at low -> too much light");
#endif
      // We are already at the lowest gain setting. If we still have an invalid value,
      // it likely means that there is too much light for our sensor.
      return ERR_OUT_OF_RANGE;
    }
  } else {
    // Good, we got a useable value.
    // Adjust it for gain factor so we end up with a single scale for all values.
#if DEBUG
    Serial.print("LOW: "); Serial.println(v);
#endif

    return Measurement::calculateLux(v, 0, tsl.getTiming(), tsl.getGain());
  }

  return MSRMT_ERROR; 
}


float read_auto_time() {
  int32_t v;

  tsl.setTiming(TSL2591_INTEGRATIONTIME_600MS);
  v = raw_read();
  if (v < 0) {
    BAIL_ON_ERROR(v)

    // Overflow. Fall through and try again with a different integration time.
  } else {     // Good, we got a useable value.
    return Measurement::calculateLux(v, 0, tsl.getTiming(), tsl.getGain());
  }

  tsl.setTiming(TSL2591_INTEGRATIONTIME_500MS);
  v = raw_read();
  if (v < 0) {
    BAIL_ON_ERROR(v)

    // Overflow. Fall through and try again with a different integration time.
  } else {     // Good, we got a useable value.
    return Measurement::calculateLux(v, 0, tsl.getTiming(), tsl.getGain());
  }

  tsl.setTiming(TSL2591_INTEGRATIONTIME_400MS);
  v = raw_read();
  if (v < 0) {
    BAIL_ON_ERROR(v)

    // Overflow. Fall through and try again with a different integration time.
  } else {     // Good, we got a useable value.
    return Measurement::calculateLux(v, 0, tsl.getTiming(), tsl.getGain());
  }

  tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);
  v = raw_read();
  if (v < 0) {
    BAIL_ON_ERROR(v)

    // Overflow. Fall through and try again with a different integration time.
  } else {     // Good, we got a useable value.
    return Measurement::calculateLux(v, 0, tsl.getTiming(), tsl.getGain());
  }

  tsl.setTiming(TSL2591_INTEGRATIONTIME_200MS);
  v = raw_read();
  if (v < 0) {
    BAIL_ON_ERROR(v)

    // Overflow. Fall through and try again with a different integration time.
  } else {     // Good, we got a useable value.
    return Measurement::calculateLux(v, 0, tsl.getTiming(), tsl.getGain());
  }

  tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);
  v = raw_read();
  if (v < 0) {
    BAIL_ON_ERROR(v)

    // We are already at the lowest integration time setting. If we still have
    // an invalid value, it means that there is too much light for our
    // sensor.
    return ERR_OUT_OF_RANGE;
  } else {     // Good, we got a useable value.
    return Measurement::calculateLux(v, 0, tsl.getTiming(), tsl.getGain());
  }

  return MSRMT_ERROR; 
}

float read_auto_gain_time() {

}

void poll_serial_and_process() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    Serial.println("[ok] " + input); // echo command back as feedback

    if (input.startsWith("read ")) {
      uint32_t num_measurements = (uint32_t)input.substring(5).toInt();
      num_measurements = num_measurements > MAX_MEASUREMENTS ? MAX_MEASUREMENTS : num_measurements;
      take_measurements(num_measurements);
    } else if (input.startsWith("info")) {
      // TODO
    } else if (input.equals("led on")) {
      isAutoLed = false;
      digitalWrite(BLUE_LED_PIN, HIGH);
    } else if (input.equals("led off")) {
      isAutoLed = false;
      digitalWrite(BLUE_LED_PIN, LOW);
    } else if (input.equals("led auto")) {
      isAutoLed = true;
    } else if (input.startsWith("gain ")) {
      isAutoGain = false; 
      String gain_str = input.substring(5);
      if (gain_str.startsWith("lo")) {
        tsl.setGain(TSL2591_GAIN_LOW);
      } else if (gain_str.startsWith("me")) {
        tsl.setGain(TSL2591_GAIN_MED);
      } else if (gain_str.startsWith("hi")) {
        tsl.setGain(TSL2591_GAIN_HIGH);
      } else if (gain_str.startsWith("ma")) {
        tsl.setGain(TSL2591_GAIN_MAX);
      } else if (gain_str.startsWith("au")) {
        isAutoGain = true; 
      } 
    } else if (input.startsWith("time ")) {
      isAutoTime = false;
      String time_str = input.substring(5);
      if (time_str.startsWith("1")) {
        tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);
      } else if (time_str.startsWith("2")) {
        tsl.setTiming(TSL2591_INTEGRATIONTIME_200MS);
      } else if (time_str.startsWith("3")) {
        tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);
      } else if (time_str.startsWith("4")) {
        tsl.setTiming(TSL2591_INTEGRATIONTIME_400MS);
      } else if (time_str.startsWith("5")) {
        tsl.setTiming(TSL2591_INTEGRATIONTIME_500MS);
      } else if (time_str.startsWith("6")) {
        tsl.setTiming(TSL2591_INTEGRATIONTIME_600MS);
      } else if (time_str.startsWith("a")) {
        isAutoTime = true;
      } 
    }
  }
}

void loop(void) {
  if (sensor_found) {
    poll_serial_and_process();
  } else {
    // If no sensor was found, just blink the LED to signal this
    digitalWrite(LED_BUILTIN, led_status);
    led_status  = (led_status + 1) % 2;
    delay(500);
  }  
}


/**************************************************************************/
/*
    Configures the gain and integration time for the TSL2591
*/
/**************************************************************************/
void configure_sensor() {
  // You can change the gain on the fly, to adapt to brighter/dimmer light situations
  //tsl.setGain(TSL2591_GAIN_LOW);    // 1x gain (bright light)
  //tsl.setGain(TSL2591_GAIN_MED);      // 25x gain
  //tsl.setGain(TSL2591_GAIN_HIGH);   // 428x gain
  tsl.setGain(TSL2591_GAIN_MAX);
  
  // Changing the integration time gives you a longer time over which to sense light
  // longer timelines are slower, but are good in very low light situtations!
  //tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);  // shortest integration time (bright light)
  //tsl.setTiming(TSL2591_INTEGRATIONTIME_200MS);
  //tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);
  //tsl.setTiming(TSL2591_INTEGRATIONTIME_400MS);
  //tsl.setTiming(TSL2591_INTEGRATIONTIME_500MS);
  //tsl.setTiming(TSL2591_INTEGRATIONTIME_600MS);  // longest integration time (dim light)
  tsl.setTiming(TSL2591_INTEGRATIONTIME_600MS);
}

int32_t raw_read() {
  if (!sensor_found)
  {
    return -1;
  }

  tsl.enable();
  tsl.write8(TSL2591_COMMAND_BIT | TSL2591_REGISTER_CONTROL, tsl.getTiming() | tsl.getGain());

  // Wait for ADC to complete
  for (uint8_t d = 0; d <= tsl.getTiming(); d++)
  {
    delay(120);
  }

  uint16_t x = tsl.read16(TSL2591_COMMAND_BIT | TSL2591_REGISTER_CHAN0_LOW);

  tsl.disable();

  if (x == 0xFFFF) {
    return ERR_OVERFLOW;
  }

  return x;
}


