#include "HX711.h"
#include <Arduino.h>

// HX711 circuit wiring (personalizzati per nostro progetto, vedi board fisica)
const int LOADCELL_DOUT_PIN = 26;
const int LOADCELL_SCK_PIN = 27;

HX711 scale;

void setup()
{
  Serial.begin(115200);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
}

void loop()
{

  if (scale.is_ready())
  {
    long reading = scale.read();
    Serial.print("HX711 reading: ");
    Serial.println(reading);
  }
  else
  {
    Serial.println("HX711 not found.");
  }

  delay(1000);
}