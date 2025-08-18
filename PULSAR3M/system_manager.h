// system_manager.h - Header complet avec toutes les fonctions
#ifndef SYSTEM_MANAGER_H
#define SYSTEM_MANAGER_H

#include "config.h"

class SystemManager {
public:
    static void loadConfiguration();
    static void saveConfiguration();
    static SystemMode getCurrentMode();
    static void setMode(SystemMode mode);
    static void initializeSelectedMode();
    static void connectToWiFi();
    static void printDebugInfo();
    
    // Fonctions WiFi optimisées et sécurisées
    static bool connectToWiFiOptimized();
    static bool connectToWiFiUltraSafe();
    static bool attemptWiFiConnection(const String& ssid, const String& password);
    static bool attemptWiFiConnectionSafe(const String& ssid, const String& password);
    static bool testInternetConnectivity();
    static bool attemptWiFiEnterpriseConnection(const String& ssid, const String& username, const String& password);  // NOUVEAU
    static bool attemptWiFiConnectionWithAuth(const WiFiCredentials& network);
    
    
private:
    static SystemMode currentMode;
    static Preferences preferences;
};

#endif