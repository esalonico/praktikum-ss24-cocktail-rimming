#ifndef ROUTES_H
#define ROUTES_H

#include <ESPAsyncWebServer.h>
#include "HX711.h"

extern const int NUM_LOAD_CELLS;

void setupRoutes(AsyncWebServer &server, HX711 scales[], int numScales);

#endif
