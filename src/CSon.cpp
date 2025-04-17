#include "CSon.h"

CSon::CSon(const i2s_pin_config_t& pinCfg, const i2s_config_t& cfg) {
  // allouer les buffers dynamiquement
  vReal   = new double[SAMPLES];
  vImag   = new double[SAMPLES];
  vMag    = new float[SAMPLES/2];
  i2sData = new int32_t[SAMPLES];
  // copier les configs
  memcpy((void*)&pinConfig, &pinCfg, sizeof(i2s_pin_config_t));
  memcpy((void*)&i2sConfig, &cfg, sizeof(i2s_config_t));
  // initialiser FFT
  FFT = ArduinoFFT<double>(vReal, vImag, SAMPLES, SAMPLING_FREQUENCY);
}

CSon::~CSon() {
  delete[] vReal;
  delete[] vImag;
  delete[] vMag;
  delete[] i2sData;
}

esp_err_t CSon::Setup() {
  // Installer et configurer le driver I2S
  result  = i2s_driver_install(I2S_NUM_0, &i2sConfig, 0, NULL);
  result |= i2s_set_pin(I2S_NUM_0, &pinConfig);
  result |= i2s_zero_dma_buffer(I2S_NUM_0);
  // initialiser l'écran SSD1306
  SSD1306Wire ecranOled(0x3c, SDA, SCL);
  ecranOled.init();
  ecranOled.clear();
  ecranOled.setFont(ArialMT_Plain_10);
  ecranOled.drawString(0, 0, "SurvSon v1.0");
  ecranOled.display();
  return result;
}

esp_err_t CSon::SamplesDmaAcquisition() {
  // Lecture I2S
  bytesRead = 0;
  result = i2s_read(I2S_NUM_0, (char*)i2sData,
                    SAMPLES * sizeof(int32_t), &bytesRead, portMAX_DELAY);
  if (result != ESP_OK) return result;

  // Préparer FFT
  for (int i = 0; i < SAMPLES; i++) {
    vReal[i] = (double)i2sData[i];
    vImag[i] = 0.0;
  }
  FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, vMag);

  // Calcul niveaux
  float sum = 0;
  niveauSonoreCrete = 0;
  for (int i = 0; i < SAMPLES/2; i++) {
    sum += vMag[i];
    if (vMag[i] > niveauSonoreCrete) niveauSonoreCrete = vMag[i];
  }
  niveauSonoreMoyen = sum / (SAMPLES/2);
  return ESP_OK;
}

StaticJsonDocument<256> CSon::toJsonDocument() const {
  StaticJsonDocument<256> doc;
  doc["moyen"] = niveauSonoreMoyen;
  doc["crete"] = niveauSonoreCrete;
  return doc;
}

void CSon::AfficherSpect(double* vReal, size_t len) {
  SSD1306Wire ecranOled(0x3c, SDA, SCL);
  ecranOled.clear();
  for (size_t i = 0; i < len/2; i++) {
    int h = (int)(vReal[i] / 10); // échelle arbitraire
    ecranOled.drawLine(i, 64, i, 64-h);
  }
  ecranOled.display();
}