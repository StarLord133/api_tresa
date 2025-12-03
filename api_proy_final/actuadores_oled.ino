#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>

// 'QR (1)', 64x64px
const unsigned char QR_OLEDQR__1_ [] PROGMEM = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xfe, 0x00, 0x0f, 0xbf, 0xf7, 0x70, 0x00, 0x7f, 0xfc, 0x00, 0x07, 0x9f, 0xe6, 0x60, 0x00, 0x3f, 
	0xfc, 0x7f, 0xc7, 0x1c, 0x07, 0xe3, 0xfe, 0x3f, 0xfc, 0xff, 0xe6, 0x18, 0x07, 0xe7, 0xff, 0x3f, 
	0xfc, 0xff, 0xe7, 0x00, 0x0f, 0xe7, 0xff, 0x3f, 0xfc, 0xe0, 0x67, 0x81, 0xff, 0xe6, 0x03, 0x3f, 
	0xfc, 0xe0, 0x67, 0x01, 0xff, 0xe6, 0x03, 0x3f, 0xfc, 0xe0, 0x66, 0x19, 0x98, 0x66, 0x03, 0x3f, 
	0xfc, 0xe0, 0x66, 0x19, 0x98, 0x66, 0x03, 0x3f, 0xfc, 0xe0, 0x67, 0xe6, 0x78, 0x66, 0x03, 0x3f, 
	0xfc, 0xe0, 0xe7, 0xe6, 0x78, 0x67, 0x07, 0x3f, 0xfc, 0xff, 0xe6, 0x67, 0x81, 0xe7, 0xff, 0x3f, 
	0xfc, 0x7f, 0xc6, 0x67, 0x81, 0xe3, 0xfe, 0x3f, 0xfc, 0x00, 0x06, 0x66, 0x66, 0x60, 0x00, 0x3f, 
	0xfe, 0x00, 0x0e, 0x66, 0x66, 0x70, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xfe, 0x7e, 0x7f, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xfe, 0x7e, 0x7f, 0xff, 0xff, 0xfc, 0x00, 0x60, 0x06, 0x01, 0x99, 0xcc, 0xff, 
	0xfe, 0x00, 0x60, 0x06, 0x01, 0x98, 0xdc, 0xff, 0xff, 0x01, 0xff, 0x9f, 0xe0, 0x18, 0x7f, 0x3f, 
	0xff, 0x01, 0xff, 0x9f, 0xe0, 0x18, 0x3f, 0x3f, 0xff, 0x06, 0x07, 0x98, 0x61, 0x86, 0x33, 0xff, 
	0xff, 0x06, 0x07, 0x98, 0x61, 0x87, 0x73, 0xff, 0xfc, 0xc1, 0xf9, 0x81, 0xe6, 0x1f, 0xff, 0xff, 
	0xfc, 0xe1, 0xf9, 0x81, 0xe6, 0x1f, 0xff, 0xff, 0xff, 0xe1, 0x80, 0x19, 0x81, 0xe0, 0x1c, 0x3f, 
	0xff, 0xc1, 0x80, 0x19, 0x81, 0xe0, 0x1c, 0x3f, 0xfe, 0x06, 0x78, 0x06, 0x7e, 0x00, 0x3f, 0xff, 
	0xfc, 0x0e, 0x78, 0x06, 0x7e, 0x00, 0x7f, 0xff, 0xfc, 0x7e, 0x60, 0x1f, 0x9f, 0x82, 0x30, 0x3f, 
	0xfc, 0xfe, 0x60, 0x1f, 0x9f, 0x86, 0x70, 0x3f, 0xfc, 0xe1, 0xf9, 0x86, 0x67, 0x8f, 0xf8, 0x3f, 
	0xfc, 0xe1, 0xf9, 0x86, 0x67, 0x9f, 0xfc, 0x3f, 0xfc, 0xe7, 0x80, 0x1e, 0x78, 0x00, 0x1e, 0x3f, 
	0xfc, 0xe7, 0x80, 0x1e, 0x78, 0x00, 0x0f, 0x3f, 0xff, 0xff, 0xfe, 0x67, 0xfe, 0x3e, 0x3f, 0x3f, 
	0xff, 0xff, 0xfe, 0x67, 0xfe, 0x7e, 0x7f, 0x3f, 0xfe, 0x00, 0x0e, 0x60, 0x06, 0x76, 0x30, 0x3f, 
	0xfc, 0x00, 0x06, 0x60, 0x06, 0x66, 0x70, 0x3f, 0xfc, 0x7f, 0xc7, 0x99, 0xe6, 0x7e, 0x1f, 0xff, 
	0xfc, 0xff, 0xe7, 0x99, 0xe6, 0x7e, 0x0f, 0xff, 0xfc, 0xe0, 0xe6, 0x78, 0xfe, 0x00, 0x0f, 0xff, 
	0xfc, 0xe0, 0x66, 0x78, 0x7e, 0x00, 0x0f, 0xff, 0xfc, 0xe0, 0x66, 0x3c, 0x74, 0x22, 0x3d, 0x7f, 
	0xfc, 0xe0, 0x66, 0x06, 0x60, 0x66, 0x7c, 0x3f, 0xfc, 0xe0, 0x66, 0x06, 0x20, 0x76, 0x3c, 0x3f, 
	0xfc, 0xe0, 0x66, 0x66, 0x1f, 0xfe, 0x73, 0x3f, 0xfc, 0xe0, 0xe6, 0x66, 0x1f, 0xfe, 0x33, 0x7f, 
	0xfc, 0xff, 0xe6, 0x66, 0x66, 0x00, 0x0c, 0xff, 0xfc, 0x7f, 0xc6, 0x66, 0x66, 0x00, 0x18, 0xff, 
	0xfc, 0x00, 0x06, 0x66, 0x19, 0xff, 0xf0, 0x3f, 0xfe, 0x00, 0x0e, 0x66, 0x19, 0xff, 0xf0, 0x7f, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

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

// --- CONFIGURACIÓN SERVOS ---
// Pines reasignados a D5-D8 como solicitó el usuario
const int servoPin1 = 14; // D5 (GPIO 14)
const int servoPin2 = 12; // D6 (GPIO 12)
const int servoPin3 = 13; // D7 (GPIO 13)
const int servoPin4 = 15; // D8 (GPIO 15) - Nota: Debe estar LOW al arrancar

Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;

WiFiClientSecure client;
HTTPClient http;

unsigned long lastCheckTime = 0;
const long checkInterval = 5000; // Consultar cada 5 segundos

// Variables globales para lógica de servos

int currentDistance = 0;

// Constantes de movimiento
const int NEUTRAL = 90;
const int STEP_SIZE = 50;  // Amplitud del paso
const int STEP_DELAY = 100; // Tiempo entre movimientos (ajustable)

void setup() {
  Serial.begin(115200);
  
  // Inicializar OLED
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

  // Inicializar Servos
  servo1.attach(servoPin1);
  servo2.attach(servoPin2);
  servo3.attach(servoPin3);
  servo4.attach(servoPin4);

  // Posición inicial
  parar();

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

// Patrón de caminata
// Posición inicial neutral
void posicionInicial() {
  servo1.write(NEUTRAL);
  servo2.write(NEUTRAL);
  servo3.write(NEUTRAL);
  servo4.write(NEUTRAL);
}

// Patrón de caminata mejorado
void caminar(int pasos, int velocidad) {
  for (int i = 0; i < pasos; i++) {
    // Paso 1: Levantar pierna derecha, inclinar pies
    servo2.write(NEUTRAL + STEP_SIZE);  // Pierna derecha levanta
    servo3.write(NEUTRAL - STEP_SIZE/2); // Pie izquierdo se inclina
    servo4.write(NEUTRAL + STEP_SIZE/2); // Pie derecho se inclina
    delay(velocidad);
    
    // Paso 2: Bajar pierna derecha, levantar pierna izquierda
    servo2.write(NEUTRAL);               // Pierna derecha baja
    servo1.write(NEUTRAL - STEP_SIZE);   // Pierna izquierda levanta
    servo3.write(NEUTRAL + STEP_SIZE/2); // Pie izquierdo se inclina opuesto
    servo4.write(NEUTRAL - STEP_SIZE/2); // Pie derecho se inclina opuesto
    delay(velocidad);
    
    // Paso 3: Volver al centro
    servo1.write(NEUTRAL);
    servo3.write(NEUTRAL);
    servo4.write(NEUTRAL);
    delay(velocidad);
  }
  
  // Volver a posición inicial
  posicionInicial();
}

// Detener servos en posición neutral
void parar() {
  posicionInicial();
}

// Dibujar QR en pantalla
void drawQR() {
  display.clearDisplay();
  // Centrar imagen de 64x64 en pantalla de 128x64
  // X = (128 - 64) / 2 = 32
  // Y = (64 - 64) / 2 = 0
  display.drawBitmap(32, 0, QR_OLEDQR__1_, 64, 64, 1);
  display.display();
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
      String showQRStr = getValue(payload, "show_qr");

      // Verificar si se debe mostrar el QR
      if (showQRStr == "true" || showQRStr == "1") {
        drawQR();
        return; // Salir de la función para mantener el QR en pantalla
      }



      // Actualizar variable global de distancia para los servos
      if (distStr != "") {
        currentDistance = distStr.toInt();
      }

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

  // Consultar API cada 5 segundos
  if (currentMillis - lastCheckTime >= checkInterval) {
    lastCheckTime = currentMillis;
    getAndDisplayData();
  }

  // Lógica de Servos (se ejecuta continuamente, pero depende de currentDistance)
  // Nota: 'caminar' tiene delays bloqueantes (2 segundos), esto afectará la frecuencia de actualización de la pantalla
  // Si la distancia está en rango, caminará un ciclo y luego volverá a checar
  
  if (currentDistance >= 50 && currentDistance <= 85) {
    caminar(2, STEP_DELAY);     // Camina si el objeto está ENTRE 50 y 85 cm
  } else {
    parar();       // Fuera del rango → se detiene
  }
}
