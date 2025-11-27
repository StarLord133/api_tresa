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

DHT dht(PIN_DHT, DHTTYPE);
WiFiClientSecure client; // Cliente seguro para HTTPS
HTTPClient http;

void setup() {
  Serial.begin(115200);
  
  // Configurar pines
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_PIR, INPUT);
  
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

void loop() {
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
    delay(2000); 
    return;
  }

  // 4. CONSTRUIR LA URL Y ENVIAR A RENDER
  if (WiFi.status() == WL_CONNECTED) {
    
    // Construimos la URL con HTTPS y el host correcto
    // Nota: Convertimos 'host' a String para poder concatenar
    String url = "https://" + String(host) + "/api/log";
    
    // Añadir parámetros
    String fullRequest = url + "?temp=" + String(t) + 
                               "&hum=" + String(h) + 
                               "&dist=" + String(distancia) + 
                               "&mov=" + String(movimiento);

    Serial.print("Enviando a Render: ");
    Serial.println(fullRequest);

    // Iniciamos la conexión segura usando el cliente 'client' que ya configuramos con setInsecure()
    http.begin(client, fullRequest); 
    
    int httpCode = http.GET();       

    if (httpCode > 0) {
      Serial.printf("[HTTPS] Codigo: %d\n", httpCode);
      String payload = http.getString();
      Serial.println("Respuesta del servidor: " + payload);
    } else {
      Serial.printf("[HTTPS] Fallo, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
    
  } else {
    Serial.println("WiFi Desconectado");
  }

  delay(60000); // Enviar datos cada 5 segundos
}
