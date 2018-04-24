#ifndef __FLUORO_HTTP_H__
#define __FLUORO_HTTP_H__

#include <ESP8266WiFi.h>
#include "FluoroRun.h"

void process_http(WiFiServer *server, Run *current_run);


#endif // __FLUORO_HTTP_H__