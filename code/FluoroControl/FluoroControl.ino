/* TSL2591 Digital Light Sensor */
/* Dynamic Range: 600M:1 */
/* Maximum Lux: 88K */

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h"

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

#define DEBUG 0
#define BLUE_LED 4

Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591); // pass in a number for the sensor identifier (for your use later)
bool sensor_found = false;

void configure_sensor(void);
void display_sensor_details(void);
int read_sensor(tsl2591Gain_t gain);
int read_sensor();
int32_t raw_read(uint8_t gain);
void stop_measurement();
void process_http();
void calibrate(uint8_t ref);
int measure();

const char* ssid     = "Fluoro";
const char* password = "VerySecure";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String output5State = "off";
String output4State = "off";

// Assign output variables to GPIO pins
const int output5 = 5;
const int output4 = 4;

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
    
//    pinMode(BLUE_LED, OUTPUT);
  } 
  else {
    Serial.println("No sensor found ... check your wiring?");
  }
}

#define ERR_NO_SENSOR         -1
#define ERR_INVALID_PARAMETER -2
#define ERR_OVERFLOW          -3
#define ERR_NOT_CALIBRATED    -4

bool led_status = 0;

#define TSL2591_GMAX_FACTOR  9876
#define TSL2591_GHIGH_FACTOR  428
#define TSL2591_GMED_FACTOR    25
#define TSL2591_GLOW_FACTOR     1

#define REF1  1
#define REF2  2

int ref1 = -1; // Reference 1 for calibration
int ref2 = -1; // Reference 2 for calibration

bool doMeasure = false;
String sampleName;
long numRep = 0;
long repCount = 0;

int32_t last_value = 0;

void loop(void) 
{
  process_http();
   
  if (sensor_found) {
    int32_t v = raw_read(TSL2591_GAIN_MAX);
    if (v < 0) {
      if (v == ERR_NO_SENSOR) {
        Serial.println("Error reading sensor: No TSL2591 sensor present");
        return;        
      } else if (v == ERR_INVALID_PARAMETER) {
        Serial.println("Error reading sensor: Invalid gain parameter");
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
#endif
      Serial.println(v);
      last_value = v;
      return;
    }

    v = raw_read(TSL2591_GAIN_HIGH);
    if (v < 0) {
      if (v == ERR_NO_SENSOR) {
        Serial.println("Error reading sensor: No TSL2591 sensor present");
        return;        
      } else if (v == ERR_INVALID_PARAMETER) {
        Serial.println("Error reading sensor: Invalid gain parameter");
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
      Serial.println(v);
      last_value = v;
      return;
    }

    v = raw_read(TSL2591_GAIN_MED);
    if (v < 0) {
      if (v == ERR_NO_SENSOR) {
        Serial.println("Error reading sensor: No TSL2591 sensor present");
        return;        
      } else if (v == ERR_INVALID_PARAMETER) {
        Serial.println("Error reading sensor: Invalid gain parameter");
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
      Serial.println(v);
      last_value = v;
      return;
    }

    v = raw_read(TSL2591_GAIN_LOW);
    if (v < 0) {
      if (v == ERR_NO_SENSOR) {
        Serial.println("Error reading sensor: No TSL2591 sensor present");
        return;        
      } else if (v == ERR_INVALID_PARAMETER) {
        Serial.println("Error reading sensor: Invalid gain parameter");
        return;
      } else {
#if DEBUG
        Serial.println("Already at low -> not enough light");
#endif
        // Overflow. Fall through and try again with a different gain setting.
      }
    } else {
      // Good, we got a useable value.
      // Adjust it for gain factor so we end up with a single scale for all values.
#if DEBUG
      Serial.print("LOW: "); Serial.print(v); Serial.println(" ");
#endif
      v *= (TSL2591_GMED_FACTOR / TSL2591_GLOW_FACTOR);
      Serial.println(v);
      last_value = v;
      return;
    }  
  } else {
    // If no sensor was found, just blink the LED to signal this
    digitalWrite(LED_BUILTIN, led_status);
    led_status  = (led_status + 1) % 2;
    delay(500);
  }  
}

void calibrate(uint8_t ref) {
  // TODO implement properly
  if (ref == REF1) {
    ref1 = 100;
  } else if (ref == REF2) {
    ref2 = 1000;
  }
}

int measure(void) {
  if (ref1 == -1 || ref2 == -1) {
    return ERR_NOT_CALIBRATED;
  }
}

// TODO separate out the webpage returned

void process_http() {
  WiFiClient client = server.available();   // Listen for incoming clients
  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    String message = "";
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            if (header.indexOf("GET /calibrate/1") >= 0) {
              Serial.println("Calibrate - Ref1");
              calibrate(REF1);
              message = "Reference point 1 was set";
            } else if (header.indexOf("GET /calibrate/2") >= 0) {
              Serial.println("Calibrate - Ref2");
              calibrate(REF2);
              message = "Reference point 2 was set";
            } else if (header.indexOf("GET /measure") >= 0) {
              Serial.println("Take measurement");
              int result = measure();
              if (result == ERR_NOT_CALIBRATED) {
                message = "At least one reference point is not set. Please calibrate the device first.";
              }
            }
            
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 20px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>Fluorometer</h1>");
            
            client.println("<p>" + message + "</p>");
            client.print("<p>Last light intensity value read: "); client.print(last_value); client.println("</p>");
            client.println("<p><a href=\"/calibrate/1\"><button class=\"button\">Calibrate - Ref1</button></a></p>");
            client.println("<p><a href=\"/calibrate/2\"><button class=\"button\">Calibrate - Ref2</button></a></p>");
            client.println("<p><a href=\"/measure\"><button class=\"button\">Take measurement</button></a></p>");
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
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
//  digitalWrite(BLUE_LED, LOW);
}

