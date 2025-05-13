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
#define SAMPLES 512               ///< Nombre total d'échantillons à traiter
#define SAMPLING_FREQUENCY 44100 ///< Fréquence d’échantillonnage en Hz
#define DMA_BUF_LEN 512           ///< Taille d’un tampon DMA (en échantillons)
#define DMA_BUF_COUNT 8           ///< Nombre de tampons DMA utilisés

/**
 * @class CSon
 * @brief Classe pour l'acquisition et le traitement du signal sonore
 * 
 * Cette classe permet de configurer l’interface I2S, de capturer des données
 * audio à l’aide du DMA, d’effectuer des transformations FFT, et d’appliquer 
 * une pondération A sur les signaux audio capturés.
 */
class CSon {
public:
    /**
     * @brief Constructeur
     * 
     * Initialise la configuration I2S et les tableaux de traitement du signal.
     */
    CSon();
    
    /**
     * @brief Configure l’interface I2S
     * 
     * Installe le pilote I2S, configure les broches et initialise les tampons.
     * @return ESP_OK en cas de succès, autre code d’erreur sinon
     */
    esp_err_t Setup();

    /**
     * @brief Acquisition de données audio via DMA
     * 
     * Lit les données I2S, met à jour les niveaux sonores, et prépare la FFT.
     * @return ESP_OK si l’acquisition a réussi
     */
    esp_err_t SamplesDmaAcquisition();
    
    /**
     * @brief Applique un filtre de pondération A
     * 
     * Transforme le signal d’entrée en signal pondéré selon la courbe A.
     * @param signal Tableau contenant le signal source
     * @param output Tableau où sera stocké le signal filtré
     * @param length Nombre d’échantillons à filtrer
     */
    void applyAWeighting(double* signal, double* output, size_t length);
    
    // Attributs accessibles publiquement
    esp_err_t result;                   ///< Code de retour de la dernière opération
    int32_t i2sData[SAMPLES];           ///< Tampon de données brutes I2S (non signées)
    double vReal[SAMPLES];              ///< Partie réelle du signal pour la FFT
    double vImag[SAMPLES];              ///< Partie imaginaire pour la FFT (initialisée à zéro)
    float niveauSonoreMoyen;            ///< Niveau sonore moyen calculé
    float niveauSonoreCrete;            ///< Niveau sonore crête détecté
    int tempsEchantillon;               ///< Temps total de l'échantillonnage (en millisecondes)

private:
    i2s_pin_config_t pinCconfig;        ///< Configuration des broches I2S (BCK, WS, DATA)
    i2s_config_t i2sConfig;             ///< Paramètres de l’interface I2S
    ArduinoFFT<double> FFT;             ///< Objet de calcul FFT (template double)
};

#endif // CSON_H