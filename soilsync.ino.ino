#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <WiFi.h>
#include <HTTPClient.h>

// Define pins and sensor type
#define DHTPIN 15             // DHT sensor data pin connected to GPIO 15
#define DHTTYPE DHT11         // Change to DHT22 if using DHT22
#define SOIL_MOISTURE_PIN 36  // Soil moisture sensor connected to GPIO 36 (VP)

// Initialize DHT sensor
DHT dht(DHTPIN, DHTTYPE);

// WiFi credentials
const char* ssid = "vivo1234";
const char* password = "12345678";

// Supabase API details
const char* supabaseUrl = "https://hevymxfroubqsxmbasbs.supabase.co";
const char* apiKey = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6ImhldnlteGZyb3VicXN4bWJhc2JzIiwicm9sZSI6ImFub24iLCJpYXQiOjE3NTI3OTUyMjQsImV4cCI6MjA2ODM3MTIyNH0.8PABBeSgcYN-aheW0rZvt5-qDveNFNIHjyfJukI6cuA";
const char* tableName = "sensor_data"; // Replace with your Supabase table name

void setup() {
  Serial.begin(115200);        // Initialize serial communication
  dht.begin();                 // Start DHT sensor
  pinMode(SOIL_MOISTURE_PIN, INPUT); // Set soil moisture sensor as input

  // Connect to WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
}

void loop() {
  // Read temperature and humidity from DHT sensor
  float temperature = dht.readTemperature(); // Celsius
  float humidity = dht.readHumidity();

  // Read soil moisture level
  int soilMoistureValue = analogRead(SOIL_MOISTURE_PIN);
  soilMoistureValue = map(soilMoistureValue, 0, 4095, 100, 0); // Convert to percentage (0-100%)

  // Check for sensor errors
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    delay(2000); // Wait and retry
    return;
  }

  // Print sensor values to Serial Monitor
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  Serial.print("Soil Moisture: ");
  Serial.print(soilMoistureValue);
  Serial.println(" %");

  // Format data as JSON
  String jsonPayload = "{";
  jsonPayload += "\"temperature\": " + String(temperature) + ",";
  jsonPayload += "\"humidity\": " + String(humidity) + ",";
  jsonPayload += "\"soil_moisture\": " + String(soilMoistureValue);
  jsonPayload += "}";

  // Send data to Supabase
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(supabaseUrl) + "/rest/v1/" + tableName;

    http.begin(url.c_str());
    http.addHeader("Content-Type", "application/json");
    http.addHeader("apikey", apiKey);
    http.addHeader("Authorization", "Bearer " + String(apiKey));

    int httpResponseCode = http.POST(jsonPayload);

    if (httpResponseCode > 0) {
      Serial.println("Data sent to Supabase!");
      Serial.println("Response: " + http.getString());
    } else {
      Serial.println("Error sending data: " + String(httpResponseCode));
    }

    http.end();
  } else {
    Serial.println("WiFi not connected. Unable to send data.");
  }

  delay(2000); // Delay for 2 seconds
}