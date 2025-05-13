/**
 * @file CSon.cpp
 * @brief Implémentation de la classe d’acquisition et de traitement du son
 */

#include "CSon.h"

CSon::CSon()
{
    // Initialisation des attributs
    niveauSonoreMoyen = 0;
    niveauSonoreCrete = 0;
    tempsEchantillon = DMA_BUF_LEN * 1000 / SAMPLING_FREQUENCY; // Calcul du temps d’échantillonnage en ms

    // Configuration des broches I2S
    this->pinCconfig = {
        .bck_io_num = 14,                  // Broche BCK (horloge des bits)
        .ws_io_num = 13,                   // Broche WS (sélection de mot)
        .data_out_num = I2S_PIN_NO_CHANGE, // Pas de sortie de données
        .data_in_num = 12                  // Broche d’entrée des données (microphone)
    };

    // Configuration de l’interface I2S
    this->i2sConfig = {
        .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX), // Mode maître, réception uniquement
        .sample_rate = SAMPLING_FREQUENCY,                 // Fréquence d’échantillonnage
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,      // Taille des échantillons
        .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT,      // Canal droit uniquement
        .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
        .intr_alloc_flags = 0,          // Niveau d’interruption
        .dma_buf_count = DMA_BUF_COUNT, // Nombre de tampons DMA
        .dma_buf_len = DMA_BUF_LEN,     // Taille de chaque tampon DMA
        .use_apll = false,
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0};

    // Initialisation de la FFT
    this->FFT = ArduinoFFT<double>(this->vReal, this->vImag, SAMPLES, SAMPLING_FREQUENCY);
}

esp_err_t CSon::Setup()
{
    // Installation du pilote I2S
    result = i2s_driver_install(I2S_NUM_0, &this->i2sConfig, 0, NULL);
    // Configuration des broches I2S
    result = i2s_set_pin(I2S_NUM_0, &this->pinCconfig);
    // Réinitialisation du tampon DMA
    result = i2s_zero_dma_buffer(I2S_NUM_0);
    return result;
}

esp_err_t CSon::SamplesDmaAcquisition()
{
    // Nombre d’octets lus depuis la mémoire DMA
    size_t bytesRead;

    // Acquisition des données audio
    result = i2s_read(I2S_NUM_0, &this->i2sData, sizeof(this->i2sData), &bytesRead, portMAX_DELAY);

    if (result == ESP_OK)
    {
        int16_t samplesRead = bytesRead / 4; // Chaque échantillon fait 32 bits (4 octets)

        if (samplesRead > 0)
        {
            float moyenne = 0;

            for (int16_t i = 0; i < samplesRead; ++i)
            {
                // Décalage de 8 bits à droite pour obtenir une valeur sur 24 bits (on ignore les 8 bits de poids faible)
                i2sData[i] = i2sData[i] >> 8;

                // Remplissage des tableaux pour la FFT
                vReal[i] = (double)i2sData[i]; // Partie réelle du signal
                vImag[i] = 0.0;                // Partie imaginaire initialisée à zéro

                // Calcul de la moyenne et du niveau crête
                moyenne += abs(i2sData[i]);
                if (abs(i2sData[i]) > niveauSonoreCrete)
                    niveauSonoreCrete = abs(i2sData[i]);
            }

            // Calcul du niveau sonore moyen
            this->niveauSonoreMoyen = moyenne / samplesRead;

            // Calcul de la FFT
            FFT.windowing(this->vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
            FFT.compute(this->vReal, this->vImag, SAMPLES, FFT_FORWARD);
            FFT.complexToMagnitude(this->vReal, this->vImag, SAMPLES);
        }
    }

    return result;
}

// Coefficients du filtre IIR pour la pondération A
const double b[] = { 0.255741125204258, -0.511482250408515, 0.255741125204258 };
const double a[] = { 1.0, -1.734725768809275, 0.766006600943264 };

/**
 * @brief Applique la pondération A sur un signal audio
 * @param signal Tableau d'entrée du signal audio (avant pondération)
 * @param output Tableau de sortie du signal audio (après pondération)
 * @param length Taille du signal
 */
void applyAWeighting(double* signal, double* output, size_t length) {
    // Variables pour les retards
    double x1 = 0.0, x2 = 0.0;
    double y1 = 0.0, y2 = 0.0;

    // Application du filtre IIR sur chaque échantillon
    for (size_t i = 0; i < length; ++i) {
        double x0 = signal[i];  // Entrée actuelle du signal
        double y0 = b[0] * x0 + b[1] * x1 + b[2] * x2 - a[1] * y1 - a[2] * y2;  // Calcul de la sortie

        // Mise à jour des retards pour la prochaine itération
        x2 = x1;
        x1 = x0;
        y2 = y1;
        y1 = y0;

        // Stocker le résultat filtré dans le tableau de sortie
        output[i] = y0;
    }
}