#include <Arduino.h>
#include <WiFi.h>
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
String password;

// WiFi network settings
IPAddress local_IP(192, 168, 0, 125); // Static IP
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

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
  password = file.readStringUntil('\n');
  password.trim();

  file.close();

  // Debug output for credentials
  Serial.println("WiFi credentials read:");
  Serial.print("SSID: ");
  Serial.println(ssid);
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

  // Set Static IP address
  if (!WiFi.config(local_IP, gateway, subnet))
  {
    Serial.println("STA Failed to configure");
  }

  // Begin Wi-Fi connection
  Serial.printf("Connecting to SSID: %s\n", ssid.c_str());
  WiFi.begin(ssid, password);

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
  char buffer[50];
  sprintf(buffer, "Local IP address (%s):", ssid.c_str());
  display.println(buffer);
  display.println(WiFi.localIP().toString());
  display.println("\nPublic endpoint:");
  display.println("https://lehre.bpm.in.tum.de/~ge54bow/cocktail_rimming/api/");
  display.display();

  unsigned long duration = millis() - startTime;
  Serial.printf("Connected to Wi-Fi in %lu ms\n", duration);
}

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
