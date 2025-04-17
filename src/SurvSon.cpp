/**
 * @file SurvSon.ino
 * @brief Programme principal pour le système de surveillance sonore
 * @author Trompier
 * @date 17/04/2025
 */

#include <Wire.h>
#include <SSD1306.h>
#include "CSon.h"
#include "CCom.h"

// Instances globales selon le diagramme de classe
SSD1306 ecranOled(0x3c, 5, 4);
CSon son;
CCom com;

// Période pour les mesures
#define PERIODE_RELEVE 10000 // période en ms

/**
 * @brief Affiche le spectre sur l'écran OLED
 * @param vRe Tableau d'amplitudes du signal fréquentiel
 */
void AfficherSpecte(double * vRe) {
  // Définir les paramètres du bargraphe
  int barWidth = 1;   // Largeur de chaque barre
  int spacing = 0;    // Espace entre les barres
  int maxHeight = 63; // Hauteur maximale des barres
  ecranOled.clear();
  
  // Dessiner le bargraphe (résolution de l'écran 128x64)
  for (int i = 1; i < 128; i++) {
    // Calculer la hauteur de la barre en fonction de sa valeur
    // Pour 512 échantillons, 128 permet de voir les fréquences jusqu'à 22kHz
    double val = vRe[i];
    int barHeight = map(val, 0, 700000, 0, maxHeight);
    
    // Dessiner la barre pleine
    ecranOled.fillRect(i * (barWidth + spacing), 64 - barHeight, barWidth, barHeight);
  }
  ecranOled.display();
}

/**
 * @brief Fonction d'initialisation exécutée une fois au démarrage
 */
void setup() {
  // Initialiser la communication série
  Serial.begin(115200);
  
  // Initialiser l'écran OLED
  ecranOled.init();
  ecranOled.clear();
  ecranOled.setFont(ArialMT_Plain_16);
  ecranOled.drawString(0, 0, "Surveillance");
  ecranOled.drawString(0, 20, "Sonore v1.0");
  ecranOled.display();
  delay(2000);
  
  // Initialiser la classe son
  son.Setup();
}

/**
 * @brief Fonction principale exécutée en boucle
 */
void loop() {
  static float niveauSonoreMoy = 0;
  static float niveauSonoreMoyenDB = 0;
  static float niveauSonoreCreteDB = 0;
  static int compteurAffichage = 0;
  
  // Réinitialiser le niveau crête au début de la période
  son.niveauSonoreCrete = 0;
  
  // Calculer le nombre d'échantillons nécessaires pour la période spécifiée
  int periodeReleve = PERIODE_RELEVE / son.tempsEchantillon;
  
  // Acquérir et traiter les échantillons audio
  for (int i = 0; i < periodeReleve; i++) {
    son.SamplesDmaAcquisition();
    
    // Mettre à jour le niveau moyen
    niveauSonoreMoy = (niveauSonoreMoy * i + son.niveauSonoreMoyen) / (i + 1);
    
    // Sortie vers Serial Plotter
    Serial.print("Moyen:");
    Serial.print(son.niveauSonoreMoyen);
    Serial.print(",Crete:");
    Serial.println(son.niveauSonoreCrete);
    
    // Afficher le spectre toutes les deux itérations
    if (compteurAffichage % 2 == 0) {
      AfficherSpecte(son.vReal);
    }
    compteurAffichage++;
    
    delay(son.tempsEchantillon);
  }
  
  // Calculer les niveaux en dB
  niveauSonoreMoyenDB = 20 * log10(niveauSonoreMoy) - 14.56;
  niveauSonoreCreteDB = 20 * log10(son.niveauSonoreCrete) - 18.474;
  
  // Afficher les niveaux sur l'écran OLED
  ecranOled.clear();
  ecranOled.setFont(ArialMT_Plain_10);
  ecranOled.drawString(0, 0, "Niveau sonore:");
  ecranOled.drawString(0, 15, "Moy: " + String(niveauSonoreMoyenDB, 1) + " dB");
  ecranOled.drawString(0, 30, "Crete: " + String(niveauSonoreCreteDB, 1) + " dB");
  ecranOled.display();
  
  // Réinitialiser pour la prochaine période
  niveauSonoreMoy = 0;
}