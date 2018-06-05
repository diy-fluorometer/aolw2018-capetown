#include "FluoroHttp.h"
#include "Arduino.h"
#include "FluoroMeasurement.h"

void process_http(WiFiServer *server, Run *current_run) {

  String request;
  WiFiClient client = server->available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    String currentLine = "";                // make a String to hold incoming data from the client
    String message = "";
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        request += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:

            if (request.indexOf("GET /calibrate/1") >= 0) {
              	// TODO implement
              	message = "Not implemented";
//              message = "Reference point 1 was set";
            } else if (request.indexOf("GET /calibrate/2") >= 0) {
            	// TODO implement
            	message = "Not implemented";
//              message = "Reference point 2 was set";
//            } else if (request.indexOf("GET /measure") >= 0) {
//              int result = current_();
//              if (result == ERR_NOT_CALIBRATED) {
//                message = "At least one reference point is not set. Please calibrate the device first.";
//              }
            } else if (request.indexOf("GET /run/start") >= 0) {
                current_run->start();
            client.println("HTTP/1.1 302 Found");
            client.println("Location: /");
            client.println("Connection: close");
            client.println();
            return;
            } else if (request.indexOf("GET /run/stop") >= 0) {
            	message = "Not implemented";            	
            }

            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            Serial.println("[process_http] Returning document to client");
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 20px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>OpenFluoro</h1>");
            
            client.println("<p>" + message + "</p>");
            uint8_t i = (current_run->idx == 0) ? 0 : current_run->idx - 1;
            client.print("<p><h1>");
            if (current_run->m[i].is_valid) {
              client.print(current_run->m[i].value);
            } else {
              client.print("(no valid value)");
            }
            client.println("</h1></p>");
//            client.println("<p><a href=\"/calibrate/1\"><button class=\"button\">Calibrate - Ref1</button></a></p>");
//            client.println("<p><a href=\"/calibrate/2\"><button class=\"button\">Calibrate - Ref2</button></a></p>");
//            client.println("<p><a href=\"/measure\"><button class=\"button\">Take measurement</button></a></p>");
            client.println("<p><a href=\"/run/start\"><button class=\"button\">Read sample</button></a></p>");
//            client.println("<p><a href=\"/test/stop\"><button class=\"button\">Stop Test</button></a></p>");

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
    // Clear the request variable
    request = "";
    // Close the connection
    client.stop();
  }   
}
