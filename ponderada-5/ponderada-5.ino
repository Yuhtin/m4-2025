#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "UbidotsEsp32Mqtt.h"

const char *WIFI_SSID = "Inteli.Iot"; 
const char *WIFI_PASS = "SENHA"; 
const char *UBIDOTS_TOKEN = "BBUS-"; 
const char *DEVICE_LABEL = "esp32"; 
const char *VARIABLE_LABEL = "dbm";
const char *CLIENT_ID = "seu_id";

Ubidots ubidots(UBIDOTS_TOKEN, CLIENT_ID);

const int PUBLISH_FREQUENCY = 3000; 
unsigned long last_publish = 0;

// --- LCD I2C ---
LiquidCrystal_I2C lcd(0x27, 16, 2);

void callback(char *topic, byte *payload, unsigned int length){
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++){
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void setup(){
  Serial.begin(115200);

  // LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Iniciando...");

  // UBIDOTS + WiFi
  ubidots.setDebug(true); 
  ubidots.connectToWifi(WIFI_SSID, WIFI_PASS);
  ubidots.setCallback(callback);
  ubidots.setup();
  ubidots.reconnect();

  delay(1000);
  lcd.clear();
}

void loop() {

  // Status Wi-Fi
  if (WiFi.status() == WL_CONNECTED) {

    // ➤ MOSTRA "Conectado"
    lcd.setCursor(0, 0);
    lcd.print("Conectado        ");

    // Lê o nível do sinal
    int32_t dBm = WiFi.RSSI();

    // Mostra no display
    lcd.setCursor(0, 1);
    lcd.printf("Sinal: %d dBm   ", dBm);

    // Envia para Ubidots no intervalo
    if (millis() - last_publish > PUBLISH_FREQUENCY) {
      ubidots.add(VARIABLE_LABEL, dBm);
      ubidots.publish(DEVICE_LABEL);

      Serial.printf("Nível de Sinal Wi-Fi: %d dBm\n", dBm);
      last_publish = millis();
    }

  } else {

    // ➤ MOSTRA "Desconectado"
    lcd.setCursor(0, 0);
    lcd.print("Desconectado     ");
    lcd.setCursor(0, 1);
    lcd.print("Sinal: --- dBm   ");

    Serial.println("Wi-Fi desconectado! Tentando reconectar...");
    WiFi.begin(WIFI_SSID, WIFI_PASS);
  }
}
