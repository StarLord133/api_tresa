#include "esp_camera.h"
#include <WiFi.h>

// <<< ESTA LÍNEA ES LO QUE LE FALTABA >>>
#include "app_httpd.h"

// Usa tu board config
#include "board_config.h"

// ============================
// CONFIGURACIÓN WIFI
// ============================
const char* ssid     = "Roborregos";
const char* password = "RoBorregos2025";

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("\n========== INICIALIZANDO ==========");

  // Configuración de la cámara (board_config + camera_pins ya lo hacen)
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;

  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;

  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;

  config.pin_pwdn  = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;

  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  config.frame_size = FRAMESIZE_QVGA;
  config.jpeg_quality = 10;
  config.fb_count = 2;

  // Inicializar
  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("ERROR: Cámara no inicializada");
    return;
  }

  Serial.println("Cámara OK");

  // ============================
  // WIFI
  // ============================
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Conectando WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("\nWiFi conectado.");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // ============================
  // INICIAR INTERFAZ OFICIAL
  // ============================
  startCameraServer();

  Serial.println("=================================");
  Serial.println("   Interfaz lista en: ");
  Serial.print("   http://");
  Serial.println(WiFi.localIP());
  Serial.println("=================================");
}

void loop() {
  // NADA: todo lo maneja el servidor oficial
}

