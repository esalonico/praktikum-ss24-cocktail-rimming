/*
 Setup your scale and start the sketch WITHOUT a weight on the scale
 Once readings are displayed, place the weight on the scale
 Press +/- or a/z to adjust the calibration_factor until the output readings match the known weight
 Arduino pin 5V -> HX711 VCC
 Arduino pin GND -> HX711 GND
*/

#include <Arduino.h>
#include "HX711.h"

HX711 scale;

float calibration_factor = -410; // To ajust
float units;
float ounces;

// Function to read an integer from Serial input
int readIntFromSerial(const char *prompt)
{
    int value = 0;
    bool validInput = false;

    while (!validInput)
    {
        Serial.println(prompt);
        while (Serial.available() == 0)
        {
            // Wait for user input
        }
        String inputString = Serial.readStringUntil('\n');
        inputString.trim(); // Remove any leading/trailing whitespace

        if (inputString.length() == 0)
        {
            Serial.println("Input cannot be empty. Please enter a valid pin number.");
            continue;
        }

        if (inputString.charAt(0) == '-')
        {
            Serial.println("Negative numbers are not valid pin numbers. Please try again.");
            continue;
        }

        bool isNumber = true;
        for (unsigned int i = 0; i < inputString.length(); i++)
        {
            if (!isDigit(inputString.charAt(i)))
            {
                isNumber = false;
                break;
            }
        }

        if (isNumber)
        {
            value = inputString.toInt();
            if (value >= 0 && value <= 39) // Assuming ESP32 has pins 0 to 39
            {
                validInput = true;
            }
            else
            {
                Serial.println("Pin number out of range. Please enter a pin between 0 and 39.");
            }
        }
        else
        {
            Serial.println("Invalid input. Please enter a numeric pin number.");
        }
    }

    return value;
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    // Variables to hold the pin numbers
    int doutPin = 0;
    int sckPin = 0;

    // Read DOUT and SCK pins from user input
    doutPin = readIntFromSerial("Please enter DOUT pin:");
    sckPin = readIntFromSerial("Please enter SCK pin:");

    // Initialize the scale with these values
    scale.begin(doutPin, sckPin);

    // Continue with the calibration as before
    Serial.println("HX711 calibration sketch");
    Serial.println("Remove all weight from scale");
    Serial.println("After readings begin, place known weight on scale");
    Serial.println("Press + or a to increase calibration factor");
    Serial.println("Press - or z to decrease calibration factor");

    scale.set_scale();
    scale.tare(); // Reset the scale to 0

    long zero_factor = scale.read_average(5); // Get a baseline reading
    Serial.print("Zero factor: ");            // This can be used to remove the need to tare the scale. Useful in permanent scale projects.
    Serial.println(zero_factor);
}

void loop()
{
    scale.set_scale(calibration_factor); // Adjust to this calibration factor

    Serial.print("Reading: ");
    units = scale.get_units(), 10;
    // sleep 0.2 seconds
    delay(200);
    ounces = units * 0.035274;
    Serial.print(units, 1);
    Serial.print(" grams");
    Serial.print("    calibration_factor: ");
    Serial.print(calibration_factor, 0);
    Serial.println();

    if (Serial.available())
    {
        char temp = Serial.read();
        if (temp == '+' || temp == 'a')
            calibration_factor += 5;
        else if (temp == '-' || temp == 'z')
            calibration_factor -= 5;
    }
}
