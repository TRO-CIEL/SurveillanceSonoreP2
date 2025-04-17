/**
 * @file CSon.h
 * @brief Sound acquisition and processing class header
 */

#ifndef CSON_H
#define CSON_H

#include "arduinoFFT.h"
#include <driver/i2s.h>

// Constants
#define SAMPLES 512             // Number of samples
#define SAMPLING_FREQUENCY 44100 // Sampling frequency in Hz
#define DMA_BUF_LEN 512         // DMA buffer size: 512 samples
#define DMA_BUF_COUNT 8         // Number of DMA buffers: 8

/**
 * @brief Class for sound acquisition and processing
 */
class CSon {
public:
    /**
     * @brief Constructor for CSon class
     */
    CSon();
    
    /**
     * @brief Setup I2S interface
     * @return ESP_OK if successful
     */
    esp_err_t Setup();
    
    /**
     * @brief Acquire samples using DMA
     * @return ESP_OK if successful
     */
    esp_err_t SamplesDmaAcquisition();
    
    /**
     * @brief Apply A-Weighting filter to the signal
     * @param signal Input signal array
     * @param output Output filtered signal array
     * @param length Length of arrays
     */
    void applyAWeighting(double* signal, double* output, size_t length);
    
    // Public attributes
    esp_err_t result;                  ///< Result of last operation
    int32_t i2sData[SAMPLES];          ///< I2S data buffer
    double vReal[SAMPLES];             ///< FFT Real part
    double vImag[SAMPLES];             ///< FFT Imaginary part
    float niveauSonoreMoyen;           ///< Average sound level
    float niveauSonoreCrete;           ///< Peak sound level
    int tempsEchantillon;              ///< Sampling time in ms

private:
    i2s_pin_config_t pinCconfig;       ///< I2S pin configuration
    i2s_config_t i2sConfig;            ///< I2S configuration
    ArduinoFFT<double> FFT;            ///< FFT object
};

#endif // CSON_H