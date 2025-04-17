#include "CCom.h"

CCom::CCom(const String& ssid, const String& pass,
           const String& serveur, const String& idDev, const String& port)
  : ssid(ssid), pass(pass), pServeur(serveur), idDevice(idDev), port(port)
{}

wl_status_t CCom::Begin() {
  WiFi.begin(ssid.c_str(), pass.c_str());
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connecté");
  return WiFi.status();
}

String CCom::getMacAddress() {
  return WiFi.macAddress();
}

String CCom::SendPostRequest(const JsonDocument& data) {
  HTTPClient http;
  String url = "http://" + pServeur + ":" + port + "/data";
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  String payload;
  serializeJson(data, payload);
  int code = http.POST(payload);
  String resp = http.getString();
  http.end();
  Serial.printf("HTTP POST %s -> %d\n", url.c_str(), code);
  return resp;
}