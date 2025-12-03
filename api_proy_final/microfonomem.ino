#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <driver/i2s.h>

// --- CONFIGURACIÓN WIFI ---
const char* ssid = "Roborregos";
const char* password = "RoBorregos2025";

// --- CONFIGURACIÓN SERVIDOR PYTHON ---
// IMPORTANTE: Cambia esto por la IP de tu PC donde corre el script Python
String serverUrl = "http://192.168.1.71:5001/upload_stream"; 

// --- CONFIGURACIÓN I2S (Micrófono INMP441) ---
// Pines proporcionados por el usuario
#define I2S_WS   25   // LRCLK / WS
#define I2S_SCK  26   // BCLK / SCK
#define I2S_SD   33   // DOUT del INMP441

#define I2S_PORT I2S_NUM_0
#define I2S_SAMPLE_RATE 16000

// --- CONFIGURACIÓN BOTÓN ---
#define BUTTON_PIN 0 // Botón BOOT del ESP32

// Buffer pequeño para streaming (no necesitamos guardar todo el audio)
const int bufferLen = 512;
int16_t sBuffer[bufferLen];

void setup() {
  Serial.begin(115200);
  
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Conectar WiFi
  WiFi.begin(ssid, password);
  Serial.print("Conectando WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Conectado");
  Serial.print("IP: "); Serial.println(WiFi.localIP());

  // Configurar I2S
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = I2S_SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = bufferLen,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };
  
  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = -1,
    .data_in_num = I2S_SD
  };

  i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pin_config);
  i2s_zero_dma_buffer(I2S_PORT);
  
  Serial.println("Sistema listo. Presiona el boton BOOT para grabar.");
}

// Función para verificar estado en el servidor
bool checkRecordingState() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    // IMPORTANTE: Cambia esto por la URL de tu backend
    // Si usas Render: https://api-tresa.onrender.com/api/recording/status
    // Si es local: http://192.168.1.71:3000/api/recording/status
    http.begin("https://api-tresa.onrender.com/api/recording/status"); 
    
    int httpCode = http.GET();
    if (httpCode > 0) {
      String payload = http.getString();
      // Buscamos "recording":true en la respuesta JSON simple
      if (payload.indexOf("\"recording\":true") >= 0) {
        http.end();
        return true;
      }
    }
    http.end();
  }
  return false;
}

void loop() {
  bool shouldRecord = false;

  // 1. Verificar botón físico
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(50); // Debounce simple
    if (digitalRead(BUTTON_PIN) == LOW) {
      shouldRecord = true;
      Serial.println("Inicio por BOTON FISICO");
    }
  }

  // 2. Verificar estado remoto (Polling cada 1s si no se presionó botón)
  if (!shouldRecord) {
    static unsigned long lastCheck = 0;
    if (millis() - lastCheck > 1000) {
      lastCheck = millis();
      if (checkRecordingState()) {
        shouldRecord = true;
        Serial.println("Inicio por COMANDO REMOTO");
      }
    }
  }

  if (shouldRecord) {
    Serial.println("Iniciando streaming...");
    
    if (WiFi.status() == WL_CONNECTED) {
      WiFiClientSecure client;
      client.setInsecure(); 
      
      String host = "api-tresa-microfono.onrender.com";
      int port = 443;
      String path = "/upload_stream";
      
      if (client.connect(host.c_str(), port)) {
        Serial.println("Conectado al servidor Python");
        
        client.println("POST " + path + " HTTP/1.1");
        client.println("Host: " + host);
        client.println("Content-Type: application/octet-stream");
        client.println("Transfer-Encoding: chunked");
        client.println("Connection: close");
        client.println();
        
        // Bucle de grabación
        // Se mantiene mientras:
        // A) El botón esté presionado (si fue iniciado por botón)
        // B) El estado remoto sea true (si fue iniciado remotamente o mixto)
        // Para simplificar, verificamos ambas condiciones periódicamente
        
        unsigned long lastRemoteCheck = 0;
        bool keepRecording = true;

        while (keepRecording) {
          size_t bytesIn = 0;
          esp_err_t result = i2s_read(I2S_PORT, &sBuffer, bufferLen * sizeof(int16_t), &bytesIn, portMAX_DELAY);
          
          if (result == ESP_OK && bytesIn > 0) {
            client.print(bytesIn, HEX);
            client.println();
            client.write((const uint8_t*)sBuffer, bytesIn);
            client.println();
          }

          // Lógica de salida
          // Si el botón está presionado, seguimos.
          // Si NO está presionado, verificamos el estado remoto.
          if (digitalRead(BUTTON_PIN) != LOW) {
             // Solo verificamos remoto cada 1s para no saturar
             if (millis() - lastRemoteCheck > 1000) {
               lastRemoteCheck = millis();
               if (!checkRecordingState()) {
                 keepRecording = false; // El servidor dijo basta
                 Serial.println("Detenido por COMANDO REMOTO");
               }
             }
          }
        }
        
        client.println("0");
        client.println();
        
        // Limpiar respuesta
        while(client.connected()) {
          String line = client.readStringUntil('\n');
          if (line == "\r") break;
        }
        client.stop();
        Serial.println("Streaming finalizado");
        
      } else {
        Serial.println("Error conexión Render");
      }
    }
    
    delay(1000); 
  }
}
