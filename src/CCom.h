/**
 * @file CCom.h
 * @brief Fichier d'en-tête pour la classe de communication WiFi
 * @author Votre Nom
 * @date 17/04/2025
 */

#ifndef CCOM_H
#define CCOM_H

#include <WiFi.h>
#include <HTTPClient.h>

/**
 * @brief Classe permettant la communication via le réseau WiFi
 * 
 * Cette classe offre une interface pour se connecter à un réseau WiFi
 * et envoyer des données via des requêtes HTTP.
 */
class CCom
{
public:
  /**
   * @brief Constructeur de la classe CCom
   * 
   * Initialise les attributs de communication.
   */
  CCom();

  /**
   * @brief Initialise la connexion WiFi
   * @return true si la connexion est réussie, false sinon
   */
  bool Setup();

  /**
   * @brief Envoie des données via HTTP (POST)
   * @param data Données à envoyer sous forme de chaîne
   * @return true si l'envoi est réussi, false sinon
   */
  bool SendData(String data);

  // Attributs publics
  String data; ///< Données à envoyer
};

#endif // CCOM_H