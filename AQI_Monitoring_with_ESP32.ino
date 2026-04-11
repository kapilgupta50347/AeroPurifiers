#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <HTTPClient.h>
#include <math.h>

// -------------------- WiFi & ThingSpeak Settings --------------------
const char* ssid = "kapil";
const char* password = "cocomelon";
String apiKey = "GKL76COYZZ04NPPU";  // Get from ThingSpeak
const char* server = "http://api.thingspeak.com/update";

// -------------------- MQ135 Pins --------------------
#define MQ135_BEFORE 34
#define MQ135_AFTER  35

// -------------------- LCD --------------------
LiquidCrystal_I2C lcd(0x27, 16, 2); // I2C address may vary (0x3F for some modules)

// -------------------- MQ135 Constants --------------------
#define RL_VALUE 10.0          // Load resistor (kΩ)
#define CALIBRATION_FACTOR 3.6 // Rs/R0 in clean air
float R0_BEFORE = 35.0;        // Calibrated R0 (kΩ)
float R0_AFTER  = 18.0;        // Calibrated R0 (kΩ)

// -------------------- Helper Functions --------------------
float getRs(int adcValue) {
  float voltage = adcValue * (3.3 / 4095.0);
  return (3.3 - voltage) * RL_VALUE / voltage;
}

float getPPM(float Rs, float R0) {
  // MQ135 characteristic equation (approx)
  return 116.6020682 * pow((Rs / R0), -2.769034857);
}

// -------------------- Setup --------------------
void setup() {
  Serial.begin(115200);
  Wire.begin(14, 27); // SDA = 14, SCL = 27
  lcd.init();
  lcd.backlight();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ESP32 Connecting");

  WiFi.begin(ssid, password);
  int attempt = 0;

  while (WiFi.status() != WL_CONNECTED && attempt < 20) {
    delay(500);
    Serial.print(".");
    attempt++;
  }

  lcd.clear();
  if (WiFi.status() == WL_CONNECTED) {
    lcd.print("ESP32 Connected!");
    Serial.println("\nWiFi connected!");
  } else {
    lcd.print("ESP32 Failed!");
    Serial.println("\nWiFi connection failed!");
  }

  delay(2000);
  lcd.clear();
}

// -------------------- Loop --------------------
void loop() {
  int adcBefore = analogRead(MQ135_BEFORE);
  int adcAfter  = analogRead(MQ135_AFTER);

  float RsBefore = getRs(adcBefore);
  float RsAfter  = getRs(adcAfter);

  float ppmBefore = getPPM(RsBefore, R0_BEFORE);
  float ppmAfter  = getPPM(RsAfter, R0_AFTER);

  int levelBefore = (int)ppmBefore;
  int levelAfter  = (int)ppmAfter;

  // Display AQI on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PPM Before:");
  lcd.print(levelBefore);
  lcd.setCursor(0, 1);
  lcd.print("PPM After:");
  lcd.print(levelAfter);

  Serial.printf("Before: %d | After: %d\n", levelBefore, levelAfter);

  // -------- Send data to ThingSpeak --------
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(server) + "?api_key=" + apiKey +
                 "&field1=" + String(levelBefore) +
                 "&field2=" + String(levelAfter);
    
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0)
      Serial.println("Data sent to ThingSpeak!");
    else
      Serial.println("Error sending data.");

    http.end();
  } else {
    Serial.println("WiFi Disconnected!");
  }

  delay(15000); // Wait 15 seconds (ThingSpeak rate limit)
}
