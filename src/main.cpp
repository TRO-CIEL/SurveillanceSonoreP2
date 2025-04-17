#include "CCom.h"
#include "CSon.h"

// Paramètres WiFi/HTTP
const String SSID      = "monSSID";
const String PASS      = "monPass";
const String SERVEUR   = "192.168.1.100";
const String ID_DEVICE = "Device01";
const String PORT      = "8080";

// Configuration I2S (ESP32)
static const i2s_pin_config_t pinConfig = {
  .bck_io_num     = 14,                // Bit clock
  .ws_io_num      = 13,                // Word select (LRCLK)
  .data_out_num   = I2S_PIN_NO_CHANGE, // Pas de sortie
  .data_in_num    = 12                 // Données entrantes
};

static const i2s_config_t i2sConfig = {
  .mode                 = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX), // maître & réception
  .sample_rate          = SAMPLING_FREQUENCY,                         // 44,1 kHz
  .bits_per_sample      = I2S_BITS_PER_SAMPLE_32BIT,                  // 32 bits
  .channel_format       = I2S_CHANNEL_FMT_ONLY_RIGHT,                 // canal droit seul
  .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
  .intr_alloc_flags     = 0,                                          // niveau interruption
  .dma_buf_count        = DMA_BUF_COUNT,                              // 8 buffers DMA
  .dma_buf_len          = DMA_BUF_LEN,                                // 512 échantillons
  .use_apll             = false,
  .tx_desc_auto_clear   = false,
  .fixed_mclk           = 0
};

// Instanciations globales (composition du programme principal)
CCom com(SSID, PASS, SERVEUR, ID_DEVICE, PORT);
CSon son(pinConfig, i2sConfig);

void setup() {
  Serial.begin(115200);
  
  // Initialisation de la communication
  com.Begin();

  // Initialisation de l’acquisition sonore (I2S + FFT)
  esp_err_t result = son.Setup();
  if (result != ESP_OK) {
    Serial.println("Erreur d'initialisation I2S !");
  }

  // Affichage de démarrage OLED (optionnel)
  // son.AfficherStartup();  // à implémenter si besoin
}

void loop() {
  // Acquisition des données sonores via DMA
  son.SamplesDmaAcquisition();

  // Envoi des données vers le serveur HTTP
  com.SendPostRequest( son.toJsonDocument() );

  // Affichage FFT (si OLED connecté)
  son.AfficherSpect(son.vReal, SAMPLES);

  delay(1000); // Pause d'une seconde entre les acquisitions
}