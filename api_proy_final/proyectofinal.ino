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
// GPIO 0 = D3
// GPIO 5 = D1
// GPIO 4 = D2
// GPIO 14 = D5
#define PIN_DHT 0      
#define DHTTYPE DHT11

#define PIN_TRIG 5     
#define PIN_ECHO 4     

#define PIN_PIR 14      
#define PIN_LED 12 // D6

DHT dht(PIN_DHT, DHTTYPE);
WiFiClientSecure client; // Cliente seguro para HTTPS
HTTPClient http;

// --- VARIABLES DE TIEMPO ---
unsigned long lastLogTime = 0;
const long logInterval = 30000; // Enviar datos a la BD cada 30 segundos
const long ledInterval = 500;   // Checar estado del LED cada 0.5 segundos

void setup() {
  Serial.begin(115200);
  
  // Configurar pines
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_PIR, INPUT);
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
  // Le dice al ESP que confíe en el certificado SSL de Render sin validarlo
  client.setInsecure(); 
}

void checkLedStatus() {
  if (WiFi.status() == WL_CONNECTED) {
    String url = "https://" + String(host) + "/api/led";
    http.begin(client, url);
    int httpCode = http.GET();
    
    if (httpCode > 0) {
       String payload = http.getString();
       // Buscamos "led":true o "led":false en la respuesta JSON
       if (payload.indexOf("\"led\":true") > 0) {
         digitalWrite(PIN_LED, HIGH); 
       } else if (payload.indexOf("\"led\":false") > 0) {
         digitalWrite(PIN_LED, LOW);
       }
    }
    http.end();
  }
}

void sendSensorData() {
  // 1. LEER TEMPERATURA Y HUMEDAD
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // 2. LEER ULTRASONICO (Distancia)
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  
  long duration = pulseIn(PIN_ECHO, HIGH, 30000); 
  float distancia = duration * 0.034 / 2; 
  if (duration == 0) distancia = 0; 

  // 3. LEER PIR (Movimiento)
  int movimiento = digitalRead(PIN_PIR); 

  // Validar lecturas del DHT
  if (isnan(h) || isnan(t)) {
    Serial.println("Error leyendo DHT!");
    return;
  }

  // 4. CONSTRUIR LA URL Y ENVIAR A RENDER
  if (WiFi.status() == WL_CONNECTED) {
    String url = "https://" + String(host) + "/api/log";
    String fullRequest = url + "?temp=" + String(t) + 
                               "&hum=" + String(h) + 
                               "&dist=" + String(distancia) + 
                               "&mov=" + String(movimiento);

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
  // Hacemos esto en cada vuelta del loop, con un pequeño delay manual si se desea, 
  // o simplemente dejamos que corra lo más rápido posible.
  // Para no saturar, usaremos un delay pequeño al final del loop.
  checkLedStatus();

  // --- TAREA LENTA: ENVIAR DATOS ---
  if (currentMillis - lastLogTime >= logInterval) {
    lastLogTime = currentMillis;
    sendSensorData();
  }

  delay(500); // Esperar 0.5s antes de volver a checar el LED
}
