#include <Arduino.h>
#include <WiFi.h>
#include <esp_eap_client.h>  // 🔧 CORRECTION: Nouveau header pour WPA2 entreprise

#ifndef SECRETS_AWS_REST_H
#define SECRETS_AWS_REST_H

// Types d'authentification WiFi supportés (avec noms uniques)
enum WifiConnectionType {
    WIFI_TYPE_PERSONAL,    // WPA2 Personnel (SSID + mot de passe)
    WIFI_TYPE_ENTERPRISE   // WPA2 Entreprise (SSID + identifiant + mot de passe)
};

// Structure WiFi étendue pour supporter l'authentification entreprise
struct WiFiCredentials {
    const char* ssid;
    const char* password;
    WifiConnectionType authType;
    const char* username;  // Pour WPA2 entreprise (nullptr pour personnel)
};

// Déclarations externes
extern const WiFiCredentials wifiNetworks[];
extern const int wifiNetworkCount;
extern const char* AWS_API_ENDPOINT;
extern const int AWS_IOT_PORT;
extern const char* AWS_CERT_CA;

#endif // SECRETS_AWS_REST_H