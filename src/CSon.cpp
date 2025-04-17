/**
 * @file CSon.cpp
 * @brief Sound acquisition and processing class implementation
 */

#include "CSon.h"

CSon::CSon() {
    // Initialize attributes
    niveauSonoreMoyen = 0;
    niveauSonoreCrete = 0;
    tempsEchantillon = DMA_BUF_LEN * 1000 / SAMPLING_FREQUENCY; // Calculate sampling time in ms
    
    // Initialize I2S pin configuration
    this->pinCconfig = {
        .bck_io_num = 14,           // BCK pin (bit clock)
        .ws_io_num = 13,            // WS pin (word select / left-right clock)
        .data_out_num = I2S_PIN_NO_CHANGE, // No data out
        .data_in_num = 12           // Data in pin (microphone data)
    };
    
    // Initialize I2S configuration
    this->i2sConfig = {
        .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX), // Master mode, receive only
        .sample_rate = SAMPLING_FREQUENCY,                 // Sampling frequency
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,      // Sample size
        .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,      // Only right channel
        .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
        .intr_alloc_flags = 0,      // Interrupt level
        .dma_buf_count = DMA_BUF_COUNT, // Number of DMA buffers
        .dma_buf_len = DMA_BUF_LEN,     // Size of DMA buffer
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0
    };
    
    // Initialize FFT
    this->FFT = ArduinoFFT<double>(this->vReal, this->vImag, SAMPLES, SAMPLING_FREQUENCY);
}

esp_err_t CSon::Setup() {
    // Install I2S driver
    result = i2s_driver_install(I2S_NUM_0, &this->i2sConfig, 0, NULL);
    // Set I2S pins
    result = i2s_set_pin(I2S_NUM_0, &this->pinCconfig);
    // Clear DMA buffer
    result = i2s_zero_dma_buffer(I2S_NUM_0);
    return result;
}

esp_err_t CSon::SamplesDmaAcquisition() {
    // Number of bytes read from DMA memory
    size_t bytesRead;
    
    // Capture audio data
    result = i2s_read(I2S_NUM_0, &this->i2sData, sizeof(this->i2sData), &bytesRead, portMAX_DELAY);
    
    if (result == ESP_OK) {
        int16_t samplesRead = bytesRead / 4; // 32-bit (4 bytes) per sample
        
        if (samplesRead > 0) {
            float mean = 0;
            
            for (int16_t i = 0; i < samplesRead; ++i) {
                // Shift right by 8 bits to get 24-bit value (discard lowest 8 bits)
                i2sData[i] = i2sData[i] >> 8;
                
                // Fill FFT real and imaginary arrays
                vReal[i] = (double)i2sData[i]; // Real part of signal
                vImag[i] = 0.0;               // Imaginary part initialized to zero
                
                // Calculate mean and peak values
                mean += abs(i2sData[i]);
                if (abs(i2sData[i]) > niveauSonoreCrete) 
                    niveauSonoreCrete = abs(i2sData[i]);
            }
            
            // Calculate average sound level
            this->niveauSonoreMoyen = mean / samplesRead;
            
            // Calculate FFT
            FFT.windowing(this->vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
            FFT.compute(this->vReal, this->vImag, SAMPLES, FFT_FORWARD);
            FFT.complexToMagnitude(this->vReal, this->vImag, SAMPLES);
        }
    }
    
    return result;
}

// A-Weighting filter implementation
void CSon::applyAWeighting(double* signal, double* output, size_t length) {
    // Coefficients for IIR filter for A-Weighting
    const double b[] = { 0.255741125204258, -0.511482250408515, 0.255741125204258 };
    const double a[] = { 1.0, -1.734725768809275, 0.766006600943264 };
    
    // Initialize variables for past filter values
    double x1 = 0.0, x2 = 0.0;
    double y1 = 0.0, y2 = 0.0;
    
    for (size_t i = 0; i < length; ++i) {
        // Calculate IIR filter
        double x0 = signal[i];
        double y0 = b[0] * x0 + b[1] * x1 + b[2] * x2 - a[1] * y1 - a[2] * y2;
        
        // Update delays
        x2 = x1;
        x1 = x0;
        y2 = y1;
        y1 = y0;
        
        // Store filtered output in output array
        output[i] = y0;
    }
}