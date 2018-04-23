/* TSL2591 Digital Light Sensor */
/* Dynamic Range: 600M:1 */
/* Maximum Lux: 88K */

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h"

#include "FluoroMeasurement.h"
#include "FluoroRun.h"
#include "FluoroHttp.h"
#include "FluoroDevice.h"

#include <ESP8266WiFi.h>

// When using Arduino Uno:
// connect SCL to analog 5
// connect SDA to analog 4
// connect Vin to 3.3-5V DC
// connect GROUND to common ground

// When using NodeMCU Amica V2 board:
// connect SCL to D1
// connect SCL to D2
// Vin to 3.3-5V
// GND to GND

#define DEBUG 1
Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591); // pass in a number for the sensor identifier (for your use later)
bool sensor_found = false;

Run current_run;

void configure_sensor(void);
void display_sensor_details(void);
int read_sensor(tsl2591Gain_t gain);
int read_sensor();
int32_t raw_read(uint8_t gain);
void stop_measurement();
void process_http();
void calibrate(uint8_t ref);

const char* ssid     = "Fluoro";
const char* password = "VerySecure";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
boolean ap_running = false;

void setup(void) 
{
  Serial.begin(9600);

  ap_running = WiFi.softAP(ssid, password);
  // TODO actual error handling please
  if (ap_running == true) {
    Serial.println("AP running");
    server.begin();
  }
  else {
    Serial.println("Starting AP failed");
  }

  if (tsl.begin()) {
    Serial.println("Found a TSL2591 sensor");
    sensor_found = true;

    display_sensor_details();
    configure_sensor();
    
    pinMode(BLUE_LED_PIN, OUTPUT);
    digitalWrite(BLUE_LED_PIN, LOW);

    current_run.idx = 0;
    current_run.active = true;
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

bool doMeasure = false;
String sampleName;
long numRep = 0;
long repCount = 0;

// Takes a measurement and returns a Measurement object with all the right
// values filled in.
// If there is an error, the Measurement object will have is_valid set to false and
// its error field set to the appropriate error code.
//
// The routine runs through all four possible gain settings, from most sensitive
// to least sensitive. This order makes sense in the context of fluorescence measurement,
// since usually, there will be very little light to work with.

void take_measurement() {
  uint32_t timestamp;

  if (current_run.idx >= MAX_MEASUREMENTS) {
#if DEBUG
    Serial.println("Done taking measurements");
#endif
//    current_run.active = false;
    current_run.idx = 0;
//    return;
  }

#if DEBUG
  Serial.print("Taking measurement #"); Serial.println(current_run.idx);
#endif

  // Turn on the blue light source
  digitalWrite(BLUE_LED_PIN, HIGH);
  
  int32_t v = raw_read(TSL2591_GAIN_MAX);
  timestamp = millis();
  current_run.m[current_run.idx].timestamp = timestamp;
  current_run.m[current_run.idx].value = v;
  if (v < 0) {
    if (v == ERR_NO_SENSOR ||
        v == ERR_INVALID_PARAMETER) {
      current_run.m[current_run.idx].is_valid = false;
      current_run.idx++;
      return;        
    } else {
#if DEBUG
      Serial.println("Falling through from MAX to HIGH");
#endif
      // Overflow. Fall through and try again with a different gain setting.
    }
  } else {
    // Good, we got a useable value.
#if DEBUG
    Serial.print("MAX: ");
    Serial.println(v);
#endif
    current_run.m[current_run.idx].is_valid = true;
    current_run.idx++;
    return;
  }


  v = raw_read(TSL2591_GAIN_HIGH);
  timestamp = millis();
  current_run.m[current_run.idx].timestamp = timestamp;
  current_run.m[current_run.idx].value = v;
  if (v < 0) {
    if (v == ERR_NO_SENSOR ||
        v == ERR_INVALID_PARAMETER) {
      current_run.m[current_run.idx].is_valid = false;
      current_run.idx++;
      return;        
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
    Serial.print("HIGH: "); Serial.print(v); Serial.print(" ");
#endif
    v *= (TSL2591_GMAX_FACTOR / TSL2591_GHIGH_FACTOR);
#if DEBUG
    Serial.println(v);
#endif
    current_run.m[current_run.idx].is_valid = true;
    current_run.m[current_run.idx].value = v;
    current_run.idx++;
    return;
  }

  v = raw_read(TSL2591_GAIN_MED);
  timestamp = millis();
  current_run.m[current_run.idx].timestamp = timestamp;
  current_run.m[current_run.idx].value = v;
  if (v < 0) {
    if (v == ERR_NO_SENSOR ||
        v == ERR_INVALID_PARAMETER) {
      current_run.m[current_run.idx].is_valid = false;
      current_run.idx++;
      return;        
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
    Serial.print("MED: "); Serial.print(v); Serial.println(" ");
#endif
    v *= (TSL2591_GHIGH_FACTOR / TSL2591_GMED_FACTOR);
#if DEBUG
    Serial.println(v);
#endif
    current_run.m[current_run.idx].is_valid = true;
    current_run.m[current_run.idx].value = v;
    current_run.idx++;
    return;
  }

  v = raw_read(TSL2591_GAIN_LOW);
  timestamp = millis();  
  current_run.m[current_run.idx].timestamp = timestamp;
  current_run.m[current_run.idx].value = v;
  if (v < 0) {
    if (v == ERR_NO_SENSOR ||
        v == ERR_INVALID_PARAMETER) {
      current_run.m[current_run.idx].is_valid = false;
      current_run.idx++;
      return;        
    } else {
#if DEBUG
      Serial.println("Already at low -> too much light");
#endif
      // We are already at the lowest gain setting. If we still have an invalid value,
      // it likely means that there is too much light for our sensor.
      current_run.m[current_run.idx].is_valid = false;
      current_run.m[current_run.idx].value = ERR_OUT_OF_RANGE;
      current_run.idx++;
      return;
    }
  } else {
    // Good, we got a useable value.
    // Adjust it for gain factor so we end up with a single scale for all values.
#if DEBUG
    Serial.print("LOW: "); Serial.print(v); Serial.println(" ");
#endif
    v *= (TSL2591_GMED_FACTOR / TSL2591_GLOW_FACTOR);
#ifdef DEBUG
    Serial.println(v);
#endif    
    current_run.m[current_run.idx].is_valid = true;
    current_run.m[current_run.idx].value = v;
    current_run.idx++;
    return;
  }  
}

void loop(void) 
{
  process_http(&server, &current_run);
  if (sensor_found) {
    if (current_run.active) {
      take_measurement();
    }
  } else {
    // If no sensor was found, just blink the LED to signal this
    digitalWrite(LED_BUILTIN, led_status);
    led_status  = (led_status + 1) % 2;
    delay(500);
  }  
}

/**************************************************************************/
/*
    Displays some basic information on this sensor from the unified
    sensor API sensor_t type (see Adafruit_Sensor for more information)
*/
/**************************************************************************/
void display_sensor_details(void)
{
  sensor_t sensor;
  tsl.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" lux");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" lux");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" lux");  
  Serial.println("------------------------------------");
  Serial.println("");
  delay(500);
}

/**************************************************************************/
/*
    Configures the gain and integration time for the TSL2591
*/
/**************************************************************************/
void configure_sensor(void)
{
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
  tsl.setTiming(TSL2591_INTEGRATIONTIME_600MS);  // longest integration time (dim light)

  /* Display the gain and integration time for reference sake */  
  //Serial.println("------------------------------------");
  //Serial.print  ("Gain:         ");
  tsl2591Gain_t gain = tsl.getGain();
  switch(gain)
  {
    case TSL2591_GAIN_LOW:
      //Serial.println("1x (Low)");
      break;
    case TSL2591_GAIN_MED:
      //Serial.println("25x (Medium)");
      break;
    case TSL2591_GAIN_HIGH:
      //Serial.println("428x (High)");
      break;
    case TSL2591_GAIN_MAX:
      //Serial.println("9876x (Max)");
      break;
  }
}

int read_sensor(tsl2591Gain_t gain)
{
  if (gain == TSL2591_GAIN_LOW ||
      gain == TSL2591_GAIN_MED ||
      gain == TSL2591_GAIN_HIGH ||
      gain == TSL2591_GAIN_MAX) {
    // if a valid value was given for gain, set it in the sensor before reading
    tsl.setGain(gain);
  } else {
        // do nothing, just leave the gain as it is currently configured
  }

  return read_sensor();
}

int read_sensor() {
  /* Get a new sensor event */
  uint16_t lumi = tsl.getLuminosity(TSL2591_VISIBLE);
  return lumi; 
  sensors_event_t event;
  tsl.getEvent(&event);
 
  if (//(event.light == 0) |
      (event.light > 4294966000.0) | 
      (event.light <-4294966000.0))
  {
    /* If event.light = 0 lux the sensor is probably saturated */
    /* and no reliable data could be generated! */
    /* if event.light is +/- 4294967040 there was a float over/underflow */
    return -1;
  }
  else
  {
    return (int)event.light;
  }
}

int32_t raw_read(uint8_t gain) {
  if (!sensor_found)
  {
    return -1;
  }

  if ((gain != TSL2591_GAIN_LOW) &&
      (gain != TSL2591_GAIN_MED) &&
      (gain != TSL2591_GAIN_HIGH) &&
      (gain != TSL2591_GAIN_MAX)) {
    return -2;      
  }

  // Enable the device
  tsl.enable();

  tsl.write8(TSL2591_COMMAND_BIT | TSL2591_REGISTER_CONTROL, tsl.getTiming() | gain);

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

void stop_measurement() {
  doMeasure = false;
  sampleName = "";
  numRep = 0;
  repCount = 0;
  digitalWrite(BLUE_LED_PIN, LOW);
}

