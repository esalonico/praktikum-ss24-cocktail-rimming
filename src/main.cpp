#include <Arduino.h>
#include <WiFi.h>
#include <esp_wpa2.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "FS.h"
#include "SPIFFS.h"
#include "HX711.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include "routes.h"

// Eduroam network credentials file path
const char *credentialsPath = "/wifi_credentials.txt";

// WiFi credentials
String ssid;
String username;
String password;

// Define the number of load cells
const int NUM_LOAD_CELLS = 3;

// HX711 instances
HX711 scales[NUM_LOAD_CELLS];

// Pins for each load cell (DOUT, SCK)
const int LOADCELL_PINS[NUM_LOAD_CELLS][2] = {
    {26, 27}, // Load cell 1: DOUT 26, SCK 27
    {25, 14}, // Load cell 2: DOUT 25, SCK 14
    {33, 12}  // Load cell 3: DOUT 33, SCK 12
};

// Calibration factors for each load cell
float calibration_factors[NUM_LOAD_CELLS] = {-410.0, -410.0, -380.0};

// Display setup
#define OLED_RESET 4
Adafruit_SH1106 display(OLED_RESET);

// Create instances of objects
HX711 scale;               // Create an instance of the HX711 class
AsyncWebServer server(80); // Create an AsyncWebServer object on port 80

// Function Prototypes
void readWiFiCredentials();
void initializeSerial();
void initializeDisplay();
void initializeScales();
void connectToWiFi();
void initializeServer();

// Function Implementations

void initializeSerial()
{
  // Initialize Serial Monitor
  Serial.begin(115200);
  delay(1000);
}

void initializeDisplay()
{
  // Initialize the OLED display at I2C address 0x3C
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
}

void initializeScales()
{
  for (int i = 0; i < NUM_LOAD_CELLS; ++i)
  {
    Serial.printf("Initializing scale %d...\n", i + 1); // TODO: remove
    scales[i].begin(LOADCELL_PINS[i][0], LOADCELL_PINS[i][1]);
    scales[i].set_scale();
    scales[i].tare();
    scales[i].set_scale(calibration_factors[i]);
    Serial.printf("Scale %d initialized with calibration factor %.2f\n", i + 1, calibration_factors[i]); // TODO: remove
  }
}

void readWiFiCredentials()
{
  if (!SPIFFS.begin(true))
  {
    Serial.println("An error occurred while mounting SPIFFS");
    return;
  }

  File file = SPIFFS.open(credentialsPath, "r");
  if (!file)
  {
    Serial.println("Failed to open credentials file");
    return;
  }

  ssid = file.readStringUntil('\n');
  ssid.trim();
  username = file.readStringUntil('\n');
  username.trim();
  password = file.readStringUntil('\n');
  password.trim();

  file.close();

  // Debug output for credentials
  Serial.println("Credentials read:");
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("Username: ");
  Serial.println(username);
}

void connectToWiFi()
{
  Serial.println("Connecting to Wi-Fi...");
  unsigned long startTime = millis();

  // Read WiFi credentials from SPIFFS
  Serial.println("Reading Wi-Fi credentials from SPIFFS");
  readWiFiCredentials();

  // Disconnect from any previous Wi-Fi connections
  Serial.println("Disconnecting from previous Wi-Fi connections");
  WiFi.disconnect(true);

  // Begin Wi-Fi connection
  Serial.printf("Connecting to SSID: %s\n", ssid.c_str());
  WiFi.begin(ssid.c_str(), WPA2_AUTH_PEAP, username.c_str(), username.c_str(), password.c_str());

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Wi-Fi is connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Display IP address on OLED
  display.println(WiFi.localIP().toString());
  display.display();

  unsigned long duration = millis() - startTime;
  Serial.printf("Connected to Wi-Fi in %lu ms\n", duration);
}

// void connectToWiFi()
// {
//   // Read WiFi credentials from SPIFFS
//   readWiFiCredentials();

//   // Disconnect from any previous WiFi connections
//   WiFi.disconnect(true);

//   // Begin WiFi connection
//   WiFi.begin(ssid.c_str(), WPA2_AUTH_PEAP, username.c_str(), username.c_str(), password.c_str());

//   // Wait for connection
//   while (WiFi.status() != WL_CONNECTED)
//   {
//     delay(500);
//     Serial.print(F("."));
//   }
//   Serial.println("");
//   Serial.println(F("WiFi is connected!"));
//   Serial.println(F("IP address set: "));
//   Serial.println(WiFi.localIP());

//   // Display IP address on OLED
//   display.println(WiFi.localIP().toString().c_str());
//   display.display();
// }

void initializeServer()
{
  // Initialize server routes
  Serial.println("Initializing server...");
  setupRoutes(server, scales, NUM_LOAD_CELLS);

  // Start the server
  server.begin();
  Serial.println("Server started on port 80");
}

void setup()
{
  initializeSerial();
  initializeDisplay();
  initializeScales();
  connectToWiFi();
  initializeServer();
}

void loop()
{
  // No code needed here since we're using AsyncWebServer
}
