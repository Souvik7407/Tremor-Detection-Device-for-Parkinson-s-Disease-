#include <Wire.h>
#include <MPU6050.h>
#include <WiFi.h>
#include <HTTPClient.h>

MPU6050 mpu;

const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// Server URL (Node-RED or Flask)
String serverURL = "http://YOUR_PC_IP:5000/data";

float ax, ay, az;
float tremorMagnitude = 0;

unsigned long lastTime = 0;
float frequencyHz = 0;
int peakCount = 0;
float lastMagnitude = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);

  Serial.println("Initializing MPU6050...");
  mpu.initialize();

  if (!mpu.testConnection()) {
    Serial.println("MPU6050 Connection Failed!");
    while (1);
  }

  Serial.println("MPU6050 Connected!");

  WiFi.begin(ssid, password);
  Serial.print("Connecting WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected!");
  Serial.println(WiFi.localIP());

  lastTime = millis();
}

void loop() {
  int16_t rawAx, rawAy, rawAz;
  mpu.getAcceleration(&rawAx, &rawAy, &rawAz);

  ax = rawAx / 16384.0;
  ay = rawAy / 16384.0;
  az = rawAz / 16384.0;

  tremorMagnitude = sqrt(ax * ax + ay * ay + az * az);

  // Detect peaks for frequency estimation
  if (tremorMagnitude > lastMagnitude + 0.05) {
    peakCount++;
  }
  lastMagnitude = tremorMagnitude;

  unsigned long currentTime = millis();

  // Every 5 seconds calculate frequency
  if (currentTime - lastTime >= 5000) {
    frequencyHz = peakCount / 5.0;
    peakCount = 0;
    lastTime = currentTime;

    Serial.print("Tremor Magnitude: ");
    Serial.print(tremorMagnitude);
    Serial.print(" | Frequency: ");
    Serial.print(frequencyHz);
    Serial.println(" Hz");

    sendDataToServer(tremorMagnitude, frequencyHz);
  }

  delay(50);
}

void sendDataToServer(float magnitude, float freq) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverURL);
    http.addHeader("Content-Type", "application/json");

    String jsonData = "{\"magnitude\": " + String(magnitude) +
                      ", \"frequency\": " + String(freq) + "}";

    int httpResponseCode = http.POST(jsonData);

    Serial.print("Server Response: ");
    Serial.println(httpResponseCode);

    http.end();
  } else {
    Serial.println("WiFi not connected");
  }
}