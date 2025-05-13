#include <Wire.h>
#include <SSD1306.h>
#include "CSon.h"
#include "CCom.h"

// Coefficients du filtre IIR pour la pondération A
const double b[] = { 0.255741125204258, -0.511482250408515, 0.255741125204258 };
const double a[] = { 1.0, -1.734725768809275, 0.766006600943264 };

// Fonction pour appliquer le filtre IIR (pondération A) sur un tableau
void applyAWeighting(double* signal, double* output, size_t length) {
  double x1 = 0.0, x2 = 0.0;
  double y1 = 0.0, y2 = 0.0;

  for (size_t i = 0; i < length; ++i) {
    double x0 = signal[i];
    double y0 = b[0] * x0 + b[1] * x1 + b[2] * x2 - a[1] * y1 - a[2] * y2;

    // Mise à jour des retards
    x2 = x1;
    x1 = x0;
    y2 = y1;
    y1 = y0;

    // Stocker la sortie filtrée dans le tableau de sortie
    output[i] = y0;
  }
}

// Instances globales des classes, selon le diagramme de classe
SSD1306 ecranOled(0x3c, 5, 4);  // Adresse I2C, broches SDA = 5, SCL = 4
CSon son;                       // Instance pour le traitement sonore
CCom com;                       // Instance pour la communication (non utilisée ici)

// Définition de la période de mesure (en millisecondes)
#define PERIODE_RELEVE 10000  // Période = 10 secondes

/**
 * @brief Affiche le spectre de fréquences sur l'écran OLED
 * @param vRe Tableau contenant les amplitudes fréquentielles (partie réelle FFT)
 */
void AfficherSpecte(double* vRe) { // Afficher Spectre
  int largeurBarre = 1;       // Largeur de chaque barre
  int espacement = 0;         // Espacement entre les barres
  int hauteurMax = 63;        // Hauteur maximale de l'écran (64 lignes)

  ecranOled.clear();          // Effacer l'écran

  // Affichage du spectre (128 colonnes disponibles sur l'écran)
  for (int i = 1; i < 128; i++) {
    double val = vRe[i];
    int hauteurBarre = map(val, 0, 700000, 0, hauteurMax);

    // Dessiner une barre verticale (à partir du bas de l'écran)
    ecranOled.fillRect(i * (largeurBarre + espacement), 64 - hauteurBarre, largeurBarre, hauteurBarre);
  }

  ecranOled.display();  // Mettre à jour l'écran
}

/**
 * @brief Fonction d'initialisation exécutée une seule fois au démarrage
 */
void setup() {
  Serial.begin(115200);       // Initialiser le port série pour le débogage

  ecranOled.init();           // Initialiser l'écran OLED
  ecranOled.clear();
  ecranOled.setFont(ArialMT_Plain_16);
  ecranOled.drawString(0, 0, "Surveillance");
  ecranOled.drawString(0, 20, "Sonore v1.0");
  ecranOled.display();
  delay(2000);                // Pause pour lecture

  son.Setup();                // Configuration de l'interface I2S
}

/**
 * @brief Boucle principale exécutée continuellement
 */
void loop() {
  static float niveauSonoreMoy = 0;
  static float niveauSonoreMoyenDB = 0;
  static float niveauSonoreCreteDB = 0;
  static int compteurAffichage = 0;

  son.niveauSonoreCrete = 0;  // Réinitialiser le niveau crête

  // Nombre de blocs à capturer pour couvrir la période de mesure
  int nombreBlocs = PERIODE_RELEVE / son.tempsEchantillon;

  // Tableau pour stocker les données filtrées
  double vRealFiltre[512]; // Assurez-vous que la taille du tableau correspond à vos données

  for (int i = 0; i < nombreBlocs; i++) {
    son.SamplesDmaAcquisition();  // Acquisition + traitement FFT

    // Appliquer la pondération A sur le signal FFT (partie réelle)
    applyAWeighting(son.vReal, vRealFiltre, 512);  // Applique le filtre sur vReal

    // Moyenne glissante du niveau sonore
    niveauSonoreMoy = (niveauSonoreMoy * i + son.niveauSonoreMoyen) / (i + 1);

    // Affichage dans le moniteur série
    Serial.print("Moyen:");
    Serial.print(son.niveauSonoreMoyen);
    Serial.print(",Crete:");
    Serial.println(son.niveauSonoreCrete);

    // Affichage graphique du spectre toutes les deux itérations
    if (compteurAffichage % 2 == 0) {
      AfficherSpecte(vRealFiltre);  // Affiche le spectre filtré
    }
    compteurAffichage++;

    delay(son.tempsEchantillon);
  }

  // Conversion des niveaux sonores en décibels (dB)
  niveauSonoreMoyenDB = 20 * log10(niveauSonoreMoy) - 14.56;
  niveauSonoreCreteDB = 20 * log10(son.niveauSonoreCrete) - 18.474;

  // Affichage des niveaux dB sur l'écran OLED
  ecranOled.clear();
  ecranOled.setFont(ArialMT_Plain_10);
  ecranOled.drawString(0, 0, "Niveau sonore:");
  ecranOled.drawString(0, 15, "Moy: " + String(niveauSonoreMoyenDB, 1) + " dB");
  ecranOled.drawString(0, 30, "Crete: " + String(niveauSonoreCreteDB, 1) + " dB");
  ecranOled.display();

  // Réinitialiser la moyenne pour le prochain cycle
  niveauSonoreMoy = 0;
}