#include <Wire.h>
#include <SSD1306.h>
#include "CSon.h"
#include "CCom.h"

// Global instances as per class diagram
SSD1306 ecranOled(0x3c, 5, 4);
CSon son;
CCom com;

// Period for measurements
#define PERIODE_RELEVE 10000 // period in ms

void AfficherSpecte(double * vRe) {
  // Display spectrum on OLED screen
  int barWidth = 1; // Bar width
  int spacing = 0; // Space between bars
  int maxHeight = 63; // Maximum bar height
  ecranOled.clear();
  
  // Draw bargraph (screen resolution 128x64)
  for (int i = 1; i < 128; i++) {
    // Calculate bar height based on its value
    // For 512 samples, 128 allows viewing frequencies up to 22kHz
    double val = vRe[i];
    int barHeight = map(val, 0, 700000, 0, maxHeight);
    
    // Draw the full bar
    ecranOled.fillRect(i * (barWidth + spacing), 64 - barHeight, barWidth, barHeight);
  }
  ecranOled.display();
}

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  
  // Initialize OLED display
  ecranOled.init();
  ecranOled.clear();
  ecranOled.setFont(ArialMT_Plain_16);
  ecranOled.drawString(0, 0, "Surveillance");
  ecranOled.drawString(0, 20, "Sonore v1.0");
  ecranOled.display();
  delay(2000);
  
  // Initialize sound class
  son.Setup();
}

void loop() {
  static float niveauSonoreMoy = 0;
  static float niveauSonoreMoyenDB = 0;
  static float niveauSonoreCreteDB = 0;
  static int displayCounter = 0;
  
  // Reset peak level at beginning of period
  son.niveauSonoreCrete = 0;
  
  // Calculate the number of samples needed for the specified period
  int periodeReleve = PERIODE_RELEVE / son.tempsEchantillon;
  
  // Acquire and process audio samples
  for (int i = 0; i < periodeReleve; i++) {
    son.SamplesDmaAcquisition();
    
    // Update average level
    niveauSonoreMoy = (niveauSonoreMoy * i + son.niveauSonoreMoyen) / (i + 1);
    
    // Output to Serial Plotter
    Serial.print("Moyen:");
    Serial.print(son.niveauSonoreMoyen);
    Serial.print(",Crete:");
    Serial.println(son.niveauSonoreCrete);
    
    // Display spectrum every other iteration
    if (displayCounter % 2 == 0) {
      AfficherSpecte(son.vReal);
    }
    displayCounter++;
    
    delay(son.tempsEchantillon);
  }
  
  // Calculate dB levels
  niveauSonoreMoyenDB = 20 * log10(niveauSonoreMoy) - 14.56;
  niveauSonoreCreteDB = 20 * log10(son.niveauSonoreCrete) - 18.474;
  
  // Display levels on OLED
  ecranOled.clear();
  ecranOled.setFont(ArialMT_Plain_10);
  ecranOled.drawString(0, 0, "Niveau sonore:");
  ecranOled.drawString(0, 15, "Moy: " + String(niveauSonoreMoyenDB, 1) + " dB");
  ecranOled.drawString(0, 30, "Crete: " + String(niveauSonoreCreteDB, 1) + " dB");
  ecranOled.display();
  
  // Reset for next period
  niveauSonoreMoy = 0;
}