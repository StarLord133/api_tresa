#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// --- CREDENCIALES WIFI ---
const char* ssid = "Tec-IoT"; 
const char* password = "spotless.magnetic.bridge"; 

// --- CONFIGURACIÓN DEL SERVIDOR ---
const char* host = "api-tresa.onrender.com";
const int httpsPort = 443; 

// --- CONFIGURACIÓN OLED ---
#define SCREEN_WIDTH 128   
#define SCREEN_HEIGHT 64   
#define OLED_RESET   -1    
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

WiFiClientSecure client;
HTTPClient http;

unsigned long lastCheckTime = 0;
const long checkInterval = 5000; // Consultar cada 5 segundos

void setup() {
  Serial.begin(115200);
  
  // Inicializar OLED
  // D2 (SDA) y D1 (SCL) son los pines I2C por defecto en NodeMCU, pero Wire.begin(D2, D1) lo asegura
  // D2 (SDA) = GPIO 4, D1 (SCL) = GPIO 5
  Wire.begin(4, 5); 

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Error: no se pudo inicializar la OLED SSD1306"));
    while (true) delay(1000);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Iniciando...");
  display.display();

  // Conectar WiFi
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");
  display.setCursor(0, 10);
  display.println("Conectando WiFi...");
  display.display();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Conectado!");
  
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("WiFi Conectado!");
  display.print("IP: ");
  display.println(WiFi.localIP());
  display.display();
  delay(2000);

  // --- IMPORTANTE PARA RENDER ---
  client.setInsecure(); 
}

// Función auxiliar para extraer valores del JSON (parseo manual simple)
String getValue(String data, String key) {
  String keyPattern = "\"" + key + "\":";
  int start = data.indexOf(keyPattern);
  if (start == -1) return "";
  
  start += keyPattern.length();
  int end = data.indexOf(",", start);
  if (end == -1) end = data.indexOf("}", start);
  
  if (end == -1) return "";
  
  return data.substring(start, end);
}

void getAndDisplayData() {
  if (WiFi.status() == WL_CONNECTED) {
    String url = "https://" + String(host) + "/api/latest";
    
    Serial.print("Consultando: ");
    Serial.println(url);

    http.begin(client, url);
    int httpCode = http.GET();

    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println("Respuesta: " + payload);

      // Parsear datos (asumiendo formato JSON plano)
      String tempStr = getValue(payload, "temperatura");
      String humStr = getValue(payload, "humedad");
      String distStr = getValue(payload, "distancia");

      // Mostrar en OLED
      display.clearDisplay();
      
      // Título
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.println("Monitor Remoto");
      display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

      if (tempStr != "" && humStr != "") {
        // Temperatura
        display.setTextSize(2);
        display.setCursor(0, 15);
        display.print(tempStr);
        display.print(" C");

        // Humedad
        display.setTextSize(1);
        display.setCursor(0, 35);
        display.print("Hum: ");
        display.print(humStr);
        display.println(" %");

        // Distancia
        display.setCursor(0, 48);
        display.print("Dist: ");
        display.print(distStr);
        display.println(" cm");
      } else {
        display.setCursor(0, 20);
        display.println("Sin datos...");
      }

      display.display();

    } else {
      Serial.printf("[HTTPS] Fallo, error: %s\n", http.errorToString(httpCode).c_str());
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Error de conexion");
      display.display();
    }
    http.end();
  }
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastCheckTime >= checkInterval) {
    lastCheckTime = currentMillis;
    getAndDisplayData();
  }
}
