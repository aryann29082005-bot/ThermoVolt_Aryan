#include "thingProperties.h"

#include <WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// ================= PINS =================
#define VOLTAGE_PIN 34
#define LED_PIN 23
#define ONE_WIRE_BUS 5

// ================= DS18B20 =================
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// ================= CONSTANT =================
const float RESISTANCE = 3.0;
const float THRESHOLD = 0.5;

// ================= WIFI CHECK =================
unsigned long lastCheck = 0;
const unsigned long interval = 5000;

// ================= WIFI RECONNECT =================
void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  Serial.println("📡 WiFi reconnecting...");

  WiFi.disconnect();
  WiFi.begin();

  unsigned long start = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - start < 8000) {
    delay(300);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi Connected");
  } else {
    Serial.println("\n⚠️ WiFi reconnect failed");
  }
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n===== IOT SYSTEM START =====");

  pinMode(LED_PIN, OUTPUT);

  // IoT Cloud init
  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  // DS18B20
  sensors.begin();

  Serial.println("🌡 DS18B20 READY");
  Serial.println("⚡ ADC READY");
  Serial.println("💡 LED READY");

  Serial.println("===========================");
}

// ================= LOOP =================
void loop() {
  ArduinoCloud.update();

  // ================= WIFI CHECK =================
  if (millis() - lastCheck > interval) {
    lastCheck = millis();

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("❌ WiFi lost");
      connectWiFi();
    } else {
      Serial.println("📶 WiFi OK");
    }
  }

  // ================= DS18B20 =================
  sensors.requestTemperatures();
  float temp = sensors.getTempCByIndex(0);

  if (temp != -127.0 && temp != 85.0) {
    temperature = temp;
  }

  // ================= VOLTAGE READ =================
  int raw = analogRead(VOLTAGE_PIN);
  float voltage = (raw / 4095.0) * 3.3;

  adcVoltage = voltage;

  // ================= CURRENT CALCULATION =================
  float current = voltage / RESISTANCE;
  current_mA = current * 1000.0;

  // ================= LED LOGIC =================
  bool ledStateLocal = (voltage >= THRESHOLD);

  digitalWrite(LED_PIN, ledStateLocal ? HIGH : LOW);
  ledState = ledStateLocal;

  // ================= SERIAL DEBUG =================
  Serial.println("------ DATA ------");
  Serial.print("Temp: "); Serial.println(temp);
  Serial.print("Voltage: "); Serial.println(voltage);
  Serial.print("Current mA: "); Serial.println(current_mA);
  Serial.print("LED: "); Serial.println(ledStateLocal ? "ON" : "OFF");
  Serial.println("------------------\n");

  delay(1000);
}