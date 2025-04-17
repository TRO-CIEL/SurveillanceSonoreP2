/**
 * @file CSon.h
 * @brief Fichier d'en-tête pour la classe d'acquisition et de traitement sonore
 * @author Votre Nom
 * @date 17/04/2025
 */

#ifndef CSON_H
#define CSON_H

#include "arduinoFFT.h"
#include <driver/i2s.h>

// Constantes
#define SAMPLES 512              ///< Nombre d'échantillons
#define SAMPLING_FREQUENCY 44100 ///< Fréquence d'échantillonnage en Hz
#define DMA_BUF_LEN 512          ///< Taille du buffer DMA: 512 échantillons
#define DMA_BUF_COUNT 8          ///< Nombre de buffers DMA: 8

/**
 * @brief Classe pour l'acquisition et le traitement du son
 * 
 * Cette classe gère l'acquisition du son via I2S et le traitement
 * du signal sonore (niveaux, FFT, pondération).
 */
class CSon {
public:
    /**
     * @brief Constructeur de la classe CSon
     * 
     * Initialise les paramètres I2S et les attributs de la classe.
     */
    CSon();
    
    /**
     * @brief Configure l'interface I2S
     * @return ESP_OK si réussi
     */
    esp_err_t Setup();
    
    /**
     * @brief Acquiert des échantillons en utilisant le DMA
     * @return ESP_OK si réussi
     */
    esp_err_t SamplesDmaAcquisition();
    
    /**
     * @brief Applique un filtre de pondération A au signal
     * @param signal Tableau du signal d'entrée
     * @param output Tableau du signal filtré en sortie
     * @param length Longueur des tableaux
     */
    void appliquerPonderationA(double* signal, double* output, size_t length);
    
    // Attributs publics
    esp_err_t result;                  ///< Résultat de la dernière opération
    int32_t i2sData[SAMPLES];          ///< Buffer de données I2S
    double vReal[SAMPLES];             ///< Partie réelle de la FFT
    double vImag[SAMPLES];             ///< Partie imaginaire de la FFT
    float niveauSonoreMoyen;           ///< Niveau sonore moyen
    float niveauSonoreCrete;           ///< Niveau sonore crête
    int tempsEchantillon;              ///< Temps d'échantillonnage en ms

private:
    i2s_pin_config_t pinCconfig;       ///< Configuration des broches I2S
    i2s_config_t i2sConfig;            ///< Configuration I2S
    ArduinoFFT<double> FFT;            ///< Objet FFT
};

#endif // CSON_H