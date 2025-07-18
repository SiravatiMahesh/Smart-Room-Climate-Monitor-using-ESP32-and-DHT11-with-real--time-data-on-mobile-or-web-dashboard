#include <WiFi.h>
#include <HTTPClient.h>
#include "DHT.h"

// --- Configuration ---
// Replace with your WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Replace with your ThingSpeak Write API Key
String apiKey = "YOUR_THINGSPEAK_API_KEY";

// DHT Sensor Setup
#define DHTPIN 4      // GPIO pin where DHT11 is connected
#define DHTTYPE DHT11 // Using the DHT11 sensor
DHT dht(DHTPIN, DHTTYPE);

// --- Main Program ---
void setup() {
  Serial.begin(115200);
  dht.begin();

  // Connect to WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Read sensor values
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature(); // Read temperature in Celsius

  // Check if any reads failed and exit early to try again.
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    delay(2000); // Wait 2 seconds before trying again
    return;
  }

  // Only send data if connected to WiFi
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Construct the ThingSpeak URL
    String url = "http://api.thingspeak.com/update?api_key=" + apiKey;
    url += "&field1=" + String(temperature, 2); // Field 1 for temperature
    url += "&field2=" + String(humidity, 2);    // Field 2 for humidity

    // Make the HTTP GET request (ThingSpeak also accepts GET for updates)
    http.begin(url);
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error on sending GET: ");
      Serial.println(httpResponseCode);
    }

    http.end(); // Free resources
  } else {
    Serial.println("WiFi Disconnected. Cannot send data.");
  }

  // ThingSpeak's free plan has a rate limit of about 15 seconds.
  // Wait 20 seconds to be safe before sending the next reading.
  delay(20000);
}
