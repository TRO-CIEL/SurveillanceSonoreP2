/**
 * @file CCom.cpp
 * @brief Implémentation de la classe de communication
 */

#include "CCom.h"

CCom::CCom() {
    // Initialisation des attributs
    data = "";
}

bool CCom::Setup() {
    // Initialisation du WiFi (à implémenter selon les besoins)
    return true;
}

bool CCom::SendData(String data) {
    // Envoi des données via HTTP (à implémenter selon les besoins)
    return true;
}