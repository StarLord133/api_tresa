#include <WiFi.h>
#include <HTTPClient.h>
#include <driver/i2s.h>
#include <esp_heap_caps.h>   // para heap_caps_malloc y ver memoria

// WiFi
const char* ssid     = "INFINITUM27AE";
const char* password = "VTHHenT32t";
const char* serverURL = "http://192.168.1.71:5001/upload_audio";

// Pines I2S para ESP32 (AJUSTA AL CABLEADO REAL)
#define I2S_WS   25   // LRCLK / WS
#define I2S_SCK  26   // BCLK / SCK
#define I2S_SD   33   // DOUT del INMP441 → GPIO33

// Configuración I2S
#define I2S_SAMPLE_RATE 16000
#define I2S_BUFFER_SIZE 512      // DMA buffer más chico
#define RECORD_TIME      3       // segundos de grabación

#define I2S_PORT I2S_NUM_0

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("Conectando WiFi...");
  WiFi.begin(ssid, password);

  int intentos = 0;
  while (WiFi.status() != WL_CONNECTED && intentos < 20) {
    delay(500);
    Serial.print(".");
    intentos++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi conectado!");
    Serial.print("IP ESP32: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nNO se pudo conectar al WiFi");
    Serial.println("Verifica SSID, contraseña y que sea red 2.4GHz");
    while (1);
  }

  Serial.println("\nMemoria libre inicial (8bit heap):");
  Serial.println(heap_caps_get_free_size(MALLOC_CAP_8BIT));

  // --- Configurar I2S ---
  Serial.println("\nIniciando INMP441...");

  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = I2S_SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, // audio/l16 = 16 bits
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = I2S_BUFFER_SIZE,
    .use_apll = false,
    .tx_desc_auto_clear = false,
    .fixed_mclk = 0
  };

  esp_err_t err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  if (err != ESP_OK) {
    Serial.printf("Error instalando I2S driver: %d\n", err);
    while (1);
  }

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = -1,    // no TX
    .data_in_num = I2S_SD
  };

  err = i2s_set_pin(I2S_PORT, &pin_config);
  if (err != ESP_OK) {
    Serial.printf("Error configurando pines I2S: %d\n", err);
    while (1);
  }

  i2s_zero_dma_buffer(I2S_PORT);

  Serial.println("INMP441 inicializado correctamente!");
  Serial.println("\n⚡ Sistema listo! Grabando automáticamente cada 10 segundos...\n");
}

void recordAndSend() {
  // 16 bits = 2 bytes por muestra
  const size_t BUFFER_SIZE = I2S_SAMPLE_RATE * RECORD_TIME * 2;

  Serial.printf("Memoria libre antes de malloc: %u bytes\n",
                heap_caps_get_free_size(MALLOC_CAP_8BIT));

  // Usamos heap_caps_malloc para garantizar memoria 8-bit
  uint8_t* audioBuffer = (uint8_t*)heap_caps_malloc(BUFFER_SIZE, MALLOC_CAP_8BIT);
  if (!audioBuffer) {
    Serial.println("Sin memoria para buffer de audio!");
    return;
  }

  Serial.printf("Grabando %d segundos...\n", RECORD_TIME);

  size_t bytes_read = 0;
  size_t total_read = 0;

  while (total_read < BUFFER_SIZE) {
    i2s_read(I2S_PORT,
             audioBuffer + total_read,
             BUFFER_SIZE - total_read,
             &bytes_read,
             portMAX_DELAY);
    total_read += bytes_read;
  }

  Serial.println("Grabación completa. Enviando al servidor...");

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverURL);
    http.addHeader("Content-Type", "audio/l16"); // 16kHz, mono, 16-bit PCM
    http.setTimeout(30000);

    int httpCode = http.POST(audioBuffer, BUFFER_SIZE);

    if (httpCode == 200) {
      String response = http.getString();
      Serial.println("\nTRANSCRIPCIÓN:");
      Serial.println(response);
      Serial.println();
    } else {
      Serial.printf("Error HTTP: %d\n", httpCode);
      if (httpCode < 0) {
        Serial.println("Verifica que el servidor esté corriendo y la IP sea correcta");
      }
    }

    http.end();
  } else {
    Serial.println("WiFi desconectado!");
  }

  free(audioBuffer);

  Serial.printf("Memoria libre después de free: %u bytes\n",
                heap_caps_get_free_size(MALLOC_CAP_8BIT));
}

void loop() {
  Serial.println("Esperando 5 segundos antes de grabar...\n");
  delay(5000);

  recordAndSend();

  Serial.println("Pausa de 10 segundos...\n");
  delay(10000);
}
