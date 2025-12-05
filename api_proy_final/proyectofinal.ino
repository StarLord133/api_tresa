#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h> 
#include <DHT.h>

// --- CREDENCIALES WIFI ---
const char* ssid = "INFINITUM0B19"; // CAMBIAR A TU RED
const char* password = "cfJCPksIhv"; // CAMBIAR A TU RED

// --- CONFIGURACIÓN DEL SERVIDOR ---
const char* host = "api-tresa.onrender.com";
const int httpsPort = 443; 

// --- PINES Y SENSORES (GPIOs) ---
#define PIN_DHT 4 // D2 (GPIO4) - Cambiado de D0 para evitar conflicto con pull-down interno     
#define DHTTYPE DHT11

#define PIN_TRIG 14 // D5   
#define PIN_ECHO 2 // D4   

#define PIN_LED 12 // D6

DHT dht(PIN_DHT, DHTTYPE);
WiFiClientSecure client; // Cliente seguro para HTTPS
HTTPClient http;

// --- VARIABLES DE TIEMPO ---
unsigned long lastDHTTime = 0;
unsigned long lastDistTime = 0;
const long dhtInterval = 30000; // Leer DHT cada 30 segundos
const long distInterval = 5000; // Leer Distancia y enviar datos cada 5 segundos

// Variables para almacenar últimos valores válidos
float lastTemp = 0.0;
float lastHum = 0.0;

void setup() {
  Serial.begin(115200);
  
  // Configurar pines
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_LED, OUTPUT);
  
  dht.begin();

  // Conectar WiFi
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Conectado!");
  Serial.print("IP: "); Serial.println(WiFi.localIP());

  // --- IMPORTANTE PARA RENDER ---
  client.setInsecure(); 

  // Lectura inicial del DHT para tener datos válidos desde el arranque
  Serial.println("Realizando lectura inicial del DHT...");
  lastTemp = dht.readTemperature();
  lastHum = dht.readHumidity();
  
  // Si falla la primera lectura, poner valores por defecto para no enviar NaN
  if (isnan(lastTemp)) lastTemp = 0.0;
  if (isnan(lastHum)) lastHum = 0.0;
}

void checkLedStatus() {
  if (WiFi.status() == WL_CONNECTED) {
    String url = "https://" + String(host) + "/api/led";
    http.begin(client, url);
    int httpCode = http.GET();
    
    if (httpCode > 0) {
       String payload = http.getString();
       if (payload.indexOf("\"led\":true") > 0) {
         digitalWrite(PIN_LED, HIGH); 
       } else if (payload.indexOf("\"led\":false") > 0) {
         digitalWrite(PIN_LED, LOW);
       }
    }
    http.end();
  }
}

void readDHT() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (!isnan(h) && !isnan(t)) {
    lastTemp = t;
    lastHum = h;
    Serial.printf("DHT Actualizado: T=%.2f H=%.2f\n", t, h);
  } else {
    Serial.println("Error leyendo DHT (manteniendo valores anteriores)");
  }
}

void readDistanceAndSend() {
  // 1. LEER ULTRASONICO (Distancia)
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  
  long duration = pulseIn(PIN_ECHO, HIGH, 30000); 
  float distancia = duration * 0.034 / 2; 
  if (duration == 0) distancia = 0; 

  Serial.printf("Distancia: %.2f cm\n", distancia);

  // 2. ENVIAR A RENDER (Usando distancia actual + últimos valores de DHT)
  if (WiFi.status() == WL_CONNECTED) {
    String url = "https://" + String(host) + "/api/log";
    String fullRequest = url + "?temp=" + String(lastTemp) + 
                               "&hum=" + String(lastHum) + 
                               "&dist=" + String(distancia);

    Serial.print("Enviando a Render: ");
    Serial.println(fullRequest);

    http.begin(client, fullRequest); 
    int httpCode = http.GET();       

    if (httpCode > 0) {
      Serial.printf("[HTTPS] Log guardado. Codigo: %d\n", httpCode);
    } else {
      Serial.printf("[HTTPS] Fallo log, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }
}

void loop() {
  unsigned long currentMillis = millis();

  // --- TAREA RÁPIDA: CHECAR LED ---
  checkLedStatus();

  // --- TAREA DHT (Cada 30s) ---
  if (currentMillis - lastDHTTime >= dhtInterval) {
    lastDHTTime = currentMillis;
    readDHT();
  }

  // --- TAREA DISTANCIA Y ENVÍO (Cada 5s) ---
  if (currentMillis - lastDistTime >= distInterval) {
    lastDistTime = currentMillis;
    readDistanceAndSend();
  }

  delay(100); // Pequeño delay para estabilidad
}
