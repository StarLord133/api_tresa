#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <driver/i2s.h>

// --- CONFIGURACIÓN WIFI ---
const char* ssid = "INFINITUM27AE";
const char* password = "VTHHenT32t";

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

void loop() {
  // Esperar a que se presione el botón (LOW)
  if (digitalRead(BUTTON_PIN) == LOW) {
    Serial.println("Boton presionado. Iniciando streaming...");
    
    if (WiFi.status() == WL_CONNECTED) {
      WiFiClient client;
      
      // Parsear URL simple (http://IP:PORT/path)
      // Asumimos formato http://192.168.1.XX:5001/upload_stream
      int port = 5001;
      String host = "192.168.1.71"; // Ajustar si cambia
      String path = "/upload_stream";
      
      // Intentar conectar
      if (client.connect(host.c_str(), port)) {
        Serial.println("Conectado al servidor Python");
        
        // Enviar cabeceras HTTP para Chunked Transfer Encoding
        client.println("POST " + path + " HTTP/1.1");
        client.println("Host: " + host);
        client.println("Content-Type: application/octet-stream");
        client.println("Transfer-Encoding: chunked");
        client.println("Connection: close");
        client.println();
        
        // Bucle de grabación mientras el botón siga presionado
        while (digitalRead(BUTTON_PIN) == LOW) {
          size_t bytesIn = 0;
          // Leer del micrófono
          esp_err_t result = i2s_read(I2S_PORT, &sBuffer, bufferLen * sizeof(int16_t), &bytesIn, portMAX_DELAY);
          
          if (result == ESP_OK && bytesIn > 0) {
            // 1. Enviar tamaño del chunk en HEX
            client.print(bytesIn, HEX);
            client.println();
            // 2. Enviar datos crudos
            client.write((const uint8_t*)sBuffer, bytesIn);
            client.println();
          }
        }
        
        // Finalizar envío (Chunk size 0)
        client.println("0");
        client.println();
        
        Serial.println("Boton soltado. Finalizando stream...");
        
        // Leer respuesta del servidor
        while(client.connected()) {
          String line = client.readStringUntil('\n');
          if (line == "\r") break; // Fin de headers
        }
        String body = client.readString();
        Serial.println("Respuesta del servidor: " + body);
        
        client.stop();
      } else {
        Serial.println("Error: No se pudo conectar al servidor Python");
      }
    } else {
      Serial.println("Error: WiFi desconectado");
    }
    
    delay(1000); // Debounce para no reiniciar inmediatamente
  }
}
