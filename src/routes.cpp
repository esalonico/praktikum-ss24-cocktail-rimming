#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "HX711.h"
#include "routes.h"

// #define ASYNCWEBSERVER_REGEX

// External Variables
extern float calibration_factors[]; // Defined in main.cpp
extern HX711 scales[];              // Defined in main.cpp
extern const int NUM_LOAD_CELLS;    // Defined in main.cpp

// Function Prototypes for Route Handlers
void handleRoot(AsyncWebServerRequest *request);
void handleGetWeight(AsyncWebServerRequest *request);
void handleGetWeightByID(AsyncWebServerRequest *request, int id);

void handleGetCalibrationFactors(AsyncWebServerRequest *request);
void handleGetCalibrationFactorByID(AsyncWebServerRequest *request, int id);
void handleSetCalibrationFactorByID(AsyncWebServerRequest *request, int id);

// Helper Functions
void sendJSONResponse(AsyncWebServerRequest *request, int statusCode, const String &jsonContent);
void sendErrorResponse(AsyncWebServerRequest *request, int statusCode, const String &errorMessage);

/* Route Handler Implementations */
void setupRoutes(AsyncWebServer &server, HX711 scales[], int numScales)
{
    /* GENERAL ROUTES */
    server.on("/", HTTP_GET, handleRoot);   

    /* SCALE ROUTES */

    // Route to handle /weight and /weight/ID
    server.on("^/weight(?:/(\\d+))?$", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        if (request->pathArg(0) == "")
        {
            // No ID provided, return all weights
            handleGetWeight(request);
        }
        else
        {
            // ID provided
            int id = request->pathArg(0).toInt();
            handleGetWeightByID(request, id);
        } });

    // Route to handle /calibration_factor and /calibration_factor/ID
    server.on("^/calibration_factor(?:/(\\d+))?$", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        if (request->pathArg(0) == "")
        {
            // No ID provided, return all calibration factors
            handleGetCalibrationFactors(request);
        }
        else
        {
            // ID provided
            int id = request->pathArg(0).toInt();
            handleGetCalibrationFactorByID(request, id);
        } });

    // Set calibration factor for a specific load cell
    server.on("^/calibration_factor(?:/(\\d+))?$", HTTP_POST, [](AsyncWebServerRequest *request)
              {
        if (request->pathArg(0) == "")
        {
            sendErrorResponse(request, 400, "Missing load cell ID");
        }
        else
        {
            int id = request->pathArg(0).toInt();
            handleSetCalibrationFactorByID(request, id);
        } });
}

// Handle root URL
void handleRoot(AsyncWebServerRequest *request)
{
    request->send(200, "text/plain", "ESP32 Web Server is Running");
}

// Handle GET request for all weights
void handleGetWeight(AsyncWebServerRequest *request)
{
    JsonDocument jsonDoc;
    JsonArray loadCells = jsonDoc["load_cells"].to<JsonArray>();

    for (int i = 0; i < NUM_LOAD_CELLS; ++i)
    {
        JsonObject cell = loadCells.add<JsonObject>();

        cell["id"] = i + 1;
        if (!scales[i].is_ready())
        {
            cell["error"] = "Load cell not connected or not detected";
        }
        else
        {
            float weight = scales[i].get_units(5); // Read average of 5 readings
            // Ensure weight is rounded to 1 decimal and no negative values
            weight = (weight <= 0) ? 0 : round(weight * 10) / 10.0;
            cell["weight"] = weight;
        }
    }

    String jsonResponse;
    serializeJson(jsonDoc, jsonResponse);
    sendJSONResponse(request, 200, jsonResponse);
}

// Handle GET request for weight by ID
void handleGetWeightByID(AsyncWebServerRequest *request, int id)
{
    if (id < 1 || id > NUM_LOAD_CELLS)
    {
        sendErrorResponse(request, 400, "Invalid load cell ID");
        return;
    }

    int index = id - 1;

    if (!scales[index].is_ready())
    {
        sendErrorResponse(request, 500, "Load cell not connected or not detected");
        return;
    }

    float weight = scales[index].get_units(5);
    // Ensure weight is rounded to 1 decimal and no negative values
    weight = (weight <= 2) ? 0 : round(weight * 10) / 10.0;

    String jsonResponse = "{\"id\": " + String(id) + ", \"weight\": " + String(weight, 1) + "}";
    sendJSONResponse(request, 200, jsonResponse);
}

// Handle GET request for calibration factor by ID
void handleGetCalibrationFactorByID(AsyncWebServerRequest *request, int id)
{
    if (id < 1 || id > NUM_LOAD_CELLS)
    {
        sendErrorResponse(request, 400, "Invalid load cell ID");
        return;
    }

    int index = id - 1;

    String jsonResponse = "{\"id\": " + String(id) + ", \"calibration_factor\": " + String(calibration_factors[index], 2) + "}";
    sendJSONResponse(request, 200, jsonResponse);
}

// Handle GET request for all calibration factors
void handleGetCalibrationFactors(AsyncWebServerRequest *request)
{
    JsonDocument jsonDoc;

    JsonArray calFactors = jsonDoc["calibration_factors"].to<JsonArray>();

    for (int i = 0; i < NUM_LOAD_CELLS; ++i)
    {
        JsonObject calFactor = calFactors.add<JsonObject>();

        calFactor["id"] = i + 1;
        calFactor["calibration_factor"] = calibration_factors[i];
    }

    String jsonResponse;
    serializeJson(jsonDoc, jsonResponse);
    sendJSONResponse(request, 200, jsonResponse);
}

// Handle POST request to set calibration factor
void handleSetCalibrationFactorByID(AsyncWebServerRequest *request, int id)
{
    if (id < 1 || id > NUM_LOAD_CELLS)
    {
        sendErrorResponse(request, 400, "Invalid load cell ID");
        return;
    }

    int index = id - 1;

    if (request->hasParam("value", true))
    {
        float newCalFactor = request->getParam("value", true)->value().toFloat();
        calibration_factors[index] = newCalFactor;

        // Update the scale's calibration factor
        scales[index].set_scale(calibration_factors[index]);

        String response = "{\"message\": \"Calibration factor updated successfully\", \"id\": " + String(id) + ", \"calibration_factor\": " + String(calibration_factors[index], 2) + "}";
        sendJSONResponse(request, 200, response);
    }
    else
    {
        sendErrorResponse(request, 400, "Missing 'value' parameter");
    }
}

/* Helper Functions */

// Send a JSON response
void sendJSONResponse(AsyncWebServerRequest *request, int statusCode, const String &jsonContent)
{
    request->send(statusCode, "application/json", jsonContent);
}

// Send an error response in JSON format
void sendErrorResponse(AsyncWebServerRequest *request, int statusCode, const String &errorMessage)
{
    JsonDocument jsonDoc;
    jsonDoc["error"] = errorMessage;
    String jsonResponse;
    serializeJson(jsonDoc, jsonResponse);
    request->send(statusCode, "application/json", jsonResponse);
}
