#pragma once
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

class CCom {
public:
  String ssid, pass;
  String pServeur, idDevice, port;

  /**
   * @brief Constructeur
   */
  CCom(const String& ssid, const String& pass,
       const String& serveur, const String& idDev, const String& port);

  /**
   * @brief Démarre la connexion WiFi
   * @return statut WiFi
   */
  wl_status_t Begin();

  /**
   * @brief Récupère l'adresse MAC
   */
  String getMacAddress();

  /**
   * @brief Envoie une requête HTTP POST avec données JSON
   * @param data JsonDocument à envoyer
   * @return réponse serveur
   */
  String SendPostRequest(const JsonDocument& data);
};
