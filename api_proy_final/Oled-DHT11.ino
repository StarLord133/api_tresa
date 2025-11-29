#include <Arduino.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>   

const char* ssid     = "*";       
const char* password = "*"; 

#define DHTPIN  D4      
#define DHTTYPE DHT11    

DHT dht(DHTPIN, DHTTYPE);

#define SCREEN_WIDTH 128   
#define SCREEN_HEIGHT 64   
#define OLED_RESET   -1    

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void conectarWiFi() {
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Conectando WiFi...");
  display.display();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi conectado!");
  Serial.print("IP local: ");
  Serial.println(WiFi.localIP());

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("WiFi conectado");
  display.setCursor(0, 16);
  display.print("IP: ");
  display.println(WiFi.localIP());
  display.display();
  delay(2500);
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println(F("Iniciando NodeMCU + DHT11 + OLED + WiFi"));

  dht.begin();

  Wire.begin(D2, D1);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Error: no se pudo inicializar la OLED SSD1306"));
    while (true) {
      delay(1000); 
    }
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("Proyecto Temp/Hum"));
  display.println(F("NodeMCU + DHT11"));
  display.display();
  delay(2000);

  conectarWiFi();
}

void loop() {
  float humedad = dht.readHumidity();
  float tempC   = dht.readTemperature();      // °C
  float tempF   = dht.readTemperature(true);  // °F

  if (isnan(humedad) || isnan(tempC)) {
    Serial.println(F("Fallo al leer del DHT11"));

    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println(F("Error leyendo"));
    display.println(F("sensor DHT11 :("));
    display.display();

    delay(2000);
    return;
  }

  Serial.print(F("Temp: "));
  Serial.print(tempC);
  Serial.print(F(" *C ("));
  Serial.print(tempF);
  Serial.print(F(" *F), Humedad: "));
  Serial.print(humedad);
  Serial.println(F(" %"));

  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(F("Lectura ambiente"));

  display.setTextSize(2);
  display.setCursor(0, 14);
  display.print(tempC, 1);
  display.println(F(" C"));

  display.setTextSize(1);
  display.setCursor(0, 38);
  display.print(F("Hum: "));
  display.print(humedad, 1);
  display.println(F(" %"));

  display.setCursor(0, 50);
  display.print(F("IP: "));
  display.print(WiFi.localIP());

  display.display();

  delay(2000);
}
