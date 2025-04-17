#pragma once
#include <Arduino.h>
#include <arduinoFFT.h>
#include <driver/i2s.h>
#include <SSD1306Wire.h>
#include <ArduinoJson.h>

#define SAMPLES             512    // Nombre d'échantillons
#define SAMPLING_FREQUENCY  44100  // Fréquence d'échantillonnage
#define DMA_BUF_LEN         512    // Taille buffer DMA
#define DMA_BUF_COUNT       8      // Nombre de buffers DMA

class CSon {
public:
  /** niveaux sonores calculés */
  float niveauSonoreMoyen;
  float niveauSonoreCrete;
  /** buffers FFT */
  double* vReal;
  double* vImag;
  /** résultat magnitude */
  float* vMag;
  /** I2S raw data */
  int32_t* i2sData;
  /** nombre d'octets lus */
  size_t bytesRead;
  /** statut I2S */
  esp_err_t result;

  /**
   * @brief Constructeur : initialise configurations I2S et FFT
   */
  CSon(const i2s_pin_config_t& pinCfg, const i2s_config_t& cfg);
  ~CSon();

  /**
   * @brief Installe et configure I2S, initialise OLED
   * @return code erreur ESP
   */
  esp_err_t Setup();

  /**
   * @brief Acquire et traite les échantillons (DMA + FFT)
   * @return code erreur ESP
   */
  esp_err_t SamplesDmaAcquisition();

  /**
   * @brief Affiche le spectre sur l'écran SSD1306
   */
  void AfficherSpect(double* vReal, size_t len);

  /**
   * @brief Transforme résultats en JsonDocument
   */
  StaticJsonDocument<256> toJsonDocument() const;
};