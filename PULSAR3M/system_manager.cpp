// system_manager.cpp - Version avec support WiFi entreprise complet
#include "system_manager.h"
#include "spi_manager.h"
#include "led_manager.h"
#include "data_manager.h"
#include "sensor_manager.h"
#include "secrets_aws_rest.h"
#include <esp_eap_client.h>  // 🔧 CORRECTION: Nouveau header pour WPA2 entreprise

SystemMode SystemManager::currentMode = MODE_CONFIG;
Preferences SystemManager::preferences;

// Variables globales (identiques)
String patientID = "";
String patientAge = "";
String patientSex = "";
String patientWeight = "";    
String patientHeight = "";    
String studyNotes = "";
String sessionDatetime = "";
String wifiSSID = "";
String wifiPassword = "";
String awsEndpoint = "";
SystemMode selectedMode = MODE_SD_RECORDING;

// Variables temporelles
unsigned long long phoneTimestampMs = 0;
unsigned long systemStartMs = 0;
String phoneTimezone = "";

// Variable pour capturer le timestamp de début de session
unsigned long long currentSessionStartTimestamp = 0;

// Variables multi-session
String currentSessionID = "";
unsigned long sessionStartTime = 0;
int sessionCounter = 0;
String continuousFilename = "";

// Buffers de données
uint32_t *greenPPGBuffer = nullptr;
uint32_t *irPPGBuffer = nullptr;    
uint32_t *redPPGBuffer = nullptr;   
uint32_t *bluePPGBuffer = nullptr;  
unsigned long *ppgTimestampBuffer = nullptr;

float *accelXBuffer = nullptr;   
float *accelYBuffer = nullptr;   
float *accelZBuffer = nullptr;   
unsigned long *accelTimestampBuffer = nullptr;

int bufferIndex = 0;
int sampleCount = 0;
int currentBufferSize = 0;

unsigned long lastSampleTime = 0;
unsigned long lastAutoSave = 0;

void SystemManager::loadConfiguration() {
    preferences.begin("watch_config", false);
    patientID = preferences.getString("patient_id", "");
    patientAge = preferences.getString("patient_age", "");
    patientSex = preferences.getString("patient_sex", "");
    patientWeight = preferences.getString("patient_weight", "");    
    patientHeight = preferences.getString("patient_height", "");    
    studyNotes = preferences.getString("study_notes", "");
    wifiSSID = preferences.getString("wifi_ssid", "");
    wifiPassword = preferences.getString("wifi_pass", "");
    awsEndpoint = String(AWS_API_ENDPOINT);
    selectedMode = (SystemMode)preferences.getInt("mode", MODE_SD_RECORDING);
    
    // Charger les données temporelles
    phoneTimestampMs = preferences.getULong64("phone_timestamp", 0);
    systemStartMs = preferences.getULong("system_start", 0);
    phoneTimezone = preferences.getString("phone_timezone", "UTC");
    
    preferences.end();
    
    Serial.println("Configuration chargee:");
    Serial.println("Patient ID: " + patientID);
    Serial.println("Mode: " + String(selectedMode));
    
    // Debug temporel
    if (phoneTimestampMs > 0) {
        Serial.printf("🕐 Timestamp téléphone: %llu ms\n", phoneTimestampMs);
        Serial.printf("🕐 Système démarré: %lu ms\n", systemStartMs);
        Serial.printf("🌍 Timezone: %s\n", phoneTimezone.c_str());
    } else {
        Serial.println("⚠️ Pas de synchronisation temporelle");
    }
}

void SystemManager::saveConfiguration() {
    preferences.begin("watch_config", false);
    preferences.putString("patient_id", patientID);
    preferences.putString("patient_age", patientAge);
    preferences.putString("patient_sex", patientSex);
    preferences.putString("patient_weight", patientWeight);    
    preferences.putString("patient_height", patientHeight);    
    preferences.putString("study_notes", studyNotes);
    preferences.putString("wifi_ssid", wifiSSID);
    preferences.putString("wifi_pass", wifiPassword);
    preferences.putString("aws_endpoint", awsEndpoint);
    preferences.putInt("mode", selectedMode);
    
    // Sauvegarder les données temporelles
    if (phoneTimestampMs > 0) {
        preferences.putULong64("phone_timestamp", phoneTimestampMs);
        preferences.putULong("system_start", systemStartMs);
        preferences.putString("phone_timezone", phoneTimezone);
        Serial.printf("🕐 Données temporelles sauvegardées: %llu ms\n", phoneTimestampMs);
    }
    
    preferences.end();
    Serial.println("Configuration sauvee");
}

SystemMode SystemManager::getCurrentMode() {
    return currentMode;
}

void SystemManager::setMode(SystemMode mode) {
    currentMode = mode;
}

void SystemManager::initializeSelectedMode() {
    Serial.println("🔄 Initialisation du mode sélectionné...");
    
    currentMode = selectedMode;
    currentSessionID = "P" + patientID + "_" + String(millis());
    sessionStartTime = millis();
    
    // Initialiser compteur de sessions
    sessionCounter = 0;
    continuousFilename = "";
    
    // Réinitialiser le timestamp de session
    currentSessionStartTimestamp = 0;
    
    // Initialiser le timestamp système si pas encore fait
    if (systemStartMs == 0) {
        systemStartMs = millis();
        Serial.printf("🕐 Système initialisé à: %lu ms\n", systemStartMs);
    }
    
    DataManager::allocateBuffers();
    
    if (currentMode == MODE_SD_RECORDING) {
        Serial.println("🔄 Initialisation mode SD UNIFIÉ");
        
        LEDManager::setState(LED_SD_RECORD);
        SPIManager::initializeSharedMode();
        DataManager::initializeSD();
        
        Serial.println("🔋 RALLUMAGE PPG pour mode acquisition SD");
        SensorManager::powerUpPPG();
        
    } else if (currentMode == MODE_WIFI_AWS) {
        Serial.println("🔄 Initialisation mode WiFi+AWS");
        
        LEDManager::setState(LED_WIFI_CONNECT);
        SPIManager::initializeSharedMode();
        
        bool wifiConnected = connectToWiFiUltraSafe();
        if (!wifiConnected) {
            Serial.println("❌ WiFi impossible - Basculement SD");
            currentMode = MODE_SD_RECORDING;
            selectedMode = MODE_SD_RECORDING;
            LEDManager::setState(LED_SD_RECORD);
            DataManager::initializeSD();
        }
        
        Serial.println("🔋 RALLUMAGE PPG pour mode acquisition AWS");
        SensorManager::powerUpPPG();
    }
    
    Serial.println("🎯 Mode initialisé: " + String(currentMode));
}

bool SystemManager::connectToWiFiUltraSafe() {
    Serial.println("🌐 === CONNEXION WIFI AVEC SUPPORT ENTREPRISE ===");
    
    bool connected = false;
    
    Serial.println("🔄 Arrêt complet WiFi...");
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    delay(2000);
    
    WiFi.setAutoReconnect(false);
    WiFi.persistent(false);
    
    Serial.println("🔄 Redémarrage mode station...");
    WiFi.mode(WIFI_STA);
    delay(1000);
    
    WiFi.setSleep(WIFI_PS_NONE);
    
    // 🔧 PRIORITÉ 1: WiFi manuel configuré via interface web
    if (wifiSSID.length() > 0 && wifiPassword.length() > 0) {
        Serial.println("🔄 Test WiFi manuel: " + wifiSSID);
        connected = attemptWiFiConnectionSafe(wifiSSID, wifiPassword);
        
        if (connected) {
            Serial.println("✅ WiFi manuel connecté: " + WiFi.localIP().toString());
        } else {
            Serial.println("❌ Échec WiFi manuel");
            WiFi.disconnect(true);
            delay(1000);
        }
    }
    
    // 🔧 PRIORITÉ 2: Réseaux prédéfinis avec authentification avancée
    if (!connected && wifiNetworkCount > 0) {
        Serial.println("🔄 Test des réseaux prédéfinis avec authentification...");
        
        for (int i = 0; i < wifiNetworkCount && !connected; i++) {
            const WiFiCredentials& network = wifiNetworks[i];
            
            Serial.printf("🔄 Test réseau %d/%d: %s (%s)\n", 
                         i+1, wifiNetworkCount, network.ssid,
                         (network.authType == WIFI_TYPE_ENTERPRISE) ? "Entreprise" : "Personnel");
            
            // 🔧 NOUVELLE LOGIQUE: Utiliser la fonction avec authentification
            connected = attemptWiFiConnectionWithAuth(network);
            
            if (connected) {
                Serial.printf("✅ Connecté au réseau: %s\n", network.ssid);
                Serial.println("📡 IP: " + WiFi.localIP().toString());
                break;
            } else {
                Serial.printf("❌ Échec: %s\n", network.ssid);
                
                // Nettoyer la config entreprise si échec
                if (network.authType == WIFI_TYPE_ENTERPRISE) {
                    esp_eap_client_clear_identity();
                    esp_eap_client_clear_username();
                    esp_eap_client_clear_password();
                }
                
                WiFi.disconnect(true);
                delay(1000);
            }
        }
    }
    
    if (connected) {
        LEDManager::setState(LED_AWS_SEND);
        Serial.println("🎯 Mode AWS activé");
        
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("✅ WiFi stable");
        }
    } else {
        Serial.println("💥 Aucune connexion WiFi possible");
        LEDManager::setState(LED_ERROR);
        
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
        delay(500);
    }
    
    return connected;
}

// 🔧 FONCTION STANDARD: Connexion simple (pour compatibilité)
bool SystemManager::attemptWiFiConnectionSafe(const String& ssid, const String& password) {
    Serial.println("📡 Connexion standard à: " + ssid);
    
    if (ssid.length() == 0) {
        Serial.println("❌ SSID vide");
        return false;
    }
    
    WiFi.disconnect();
    delay(1000);
    
    WiFi.begin(ssid.c_str(), password.c_str());
    
    unsigned long startTime = millis();
    const unsigned long timeout = 15000;
    bool connected = false;
    
    Serial.print("🔄 Connexion");
    
    while (!connected && (millis() - startTime) < timeout) {
        wl_status_t status = WiFi.status();
        
        if (status == WL_CONNECTED) {
            connected = true;
            break;
        } else if (status == WL_CONNECT_FAILED || 
                  status == WL_NO_SSID_AVAIL ||
                  status == WL_CONNECTION_LOST) {
            Serial.println("\n❌ Échec rapide détecté: " + String(status));
            break;
        }
        
        delay(500);
        Serial.print(".");
        yield();
        
        if ((millis() - startTime) % 2000 == 0) {
            Serial.printf("\n🔍 Status: %d, Temps: %lu ms\n", status, millis() - startTime);
        }
    }
    
    if (connected) {
        Serial.println("\n✅ Connexion réussie!");
        Serial.printf("📊 IP: %s, Signal: %d dBm\n", 
                     WiFi.localIP().toString().c_str(), WiFi.RSSI());
        delay(1000);
    } else {
        Serial.println("\n❌ Timeout ou échec connexion");
        WiFi.disconnect();
        delay(500);
    }
    
    return connected;
}

// 🔧 NOUVELLE FONCTION: Connexion avec authentification avancée
bool SystemManager::attemptWiFiConnectionWithAuth(const WiFiCredentials& network) {
    Serial.printf("📡 Connexion avec auth à: %s (Type: %s)\n", 
                 network.ssid, 
                 (network.authType == WIFI_TYPE_ENTERPRISE) ? "Entreprise" : "Personnel");
    
    if (strlen(network.ssid) == 0) {
        Serial.println("❌ SSID vide");
        return false;
    }
    
    WiFi.disconnect();
    delay(1000);
    
    // Configuration selon le type d'authentification
    if (network.authType == WIFI_TYPE_ENTERPRISE && network.username != nullptr) {
        Serial.println("🏢 Configuration WPA2 Entreprise...");
        
        // 🔧 NOUVELLE API: Configuration WPA2 Enterprise
        WiFi.mode(WIFI_STA);
        
        // Utiliser les nouvelles fonctions esp_eap_client
        esp_eap_client_set_identity((uint8_t *)network.username, strlen(network.username));
        esp_eap_client_set_username((uint8_t *)network.username, strlen(network.username));
        esp_eap_client_set_password((uint8_t *)network.password, strlen(network.password));
        
        // Activer EAP
        esp_wifi_sta_enterprise_enable();
        
        WiFi.begin(network.ssid);
        Serial.printf("🏢 Connexion entreprise: %s / %s\n", network.ssid, network.username);
        
    } else {
        Serial.println("🏠 Configuration WPA2 Personnel...");
        WiFi.begin(network.ssid, network.password);
        Serial.printf("🏠 Connexion personnelle: %s\n", network.ssid);
    }
    
    unsigned long startTime = millis();
    const unsigned long timeout = 20000;  // Plus long pour entreprise
    bool connected = false;
    
    Serial.print("🔄 Connexion");
    
    while (!connected && (millis() - startTime) < timeout) {
        wl_status_t status = WiFi.status();
        
        if (status == WL_CONNECTED) {
            connected = true;
            break;
        } else if (status == WL_CONNECT_FAILED || 
                  status == WL_NO_SSID_AVAIL ||
                  status == WL_CONNECTION_LOST) {
            Serial.println("\n❌ Échec rapide détecté: " + String(status));
            break;
        }
        
        delay(500);
        Serial.print(".");
        yield();
        
        if ((millis() - startTime) % 3000 == 0) {
            Serial.printf("\n🔍 Status: %d, Temps: %lu ms\n", status, millis() - startTime);
        }
    }
    
    if (connected) {
        Serial.println("\n✅ Connexion réussie!");
        Serial.printf("📊 Type: %s\n", (network.authType == WIFI_TYPE_ENTERPRISE) ? "Entreprise" : "Personnel");
        Serial.printf("📊 IP: %s, Signal: %d dBm\n", 
                     WiFi.localIP().toString().c_str(), WiFi.RSSI());
        delay(1000);
    } else {
        Serial.println("\n❌ Timeout ou échec connexion");
        
        // Nettoyer la config entreprise si échec
        if (network.authType == WIFI_TYPE_ENTERPRISE) {
            Serial.println("🏢 Nettoyage config entreprise après échec");
            esp_eap_client_clear_identity();
            esp_eap_client_clear_username();
            esp_eap_client_clear_password();
            esp_wifi_sta_enterprise_disable();
        }
        
        WiFi.disconnect();
        delay(500);
    }
    
    return connected;
}

bool SystemManager::testInternetConnectivity() {
    if (WiFi.status() != WL_CONNECTED) {
        return false;
    }
    
    Serial.println("🌐 Test connectivité Internet...");
    
    WiFiClient client;
    bool result = false;
    
    if (client.connect("www.google.com", 80)) {
        client.println("GET / HTTP/1.1");
        client.println("Host: www.google.com");
        client.println("Connection: close");
        client.println();
        
        unsigned long timeout = millis();
        while (client.available() == 0) {
            if (millis() - timeout > 3000) {
                Serial.println("❌ Timeout test Internet");
                break;
            }
            delay(10);
        }
        
        if (client.available()) {
            Serial.println("✅ Internet accessible");
            result = true;
        }
        
        client.stop();
    } else {
        Serial.println("❌ Impossible de se connecter à Internet");
    }
    
    return result;
}

// Fonctions de compatibilité
bool SystemManager::connectToWiFiOptimized() {
    return connectToWiFiUltraSafe();
}

bool SystemManager::attemptWiFiConnection(const String& ssid, const String& password) {
    return attemptWiFiConnectionSafe(ssid, password);
}

void SystemManager::connectToWiFi() {
    connectToWiFiUltraSafe();
}

void SystemManager::printDebugInfo() {
    Serial.printf("Mode: %s, Sessions: %d, Buffer: %d/%d\n", 
                 (currentMode == MODE_SD_RECORDING) ? "SD" : 
                 (currentMode == MODE_WIFI_AWS) ? "AWS" : "CONFIG",
                 sessionCounter, bufferIndex, currentBufferSize);
                 
    if (psramFound()) {
        Serial.printf("💾 PSRAM libre: %d KB, RAM interne: %d KB\n", 
                     ESP.getFreePsram() / 1024, ESP.getFreeHeap() / 1024);
    }
    
    if (currentMode == MODE_WIFI_AWS) {
        Serial.printf("📡 WiFi Status: %d, IP: %s\n", 
                     WiFi.status(), WiFi.localIP().toString().c_str());
    }
    
    // Debug temporel
    if (phoneTimestampMs > 0) {
        unsigned long currentSystemTime = millis();
        unsigned long elapsedSinceStart = currentSystemTime - systemStartMs;
        unsigned long long estimatedCurrentTime = phoneTimestampMs + elapsedSinceStart;
        
        Serial.printf("🕐 Temps estimé actuel: %llu ms (soit %llu secondes)\n", 
                     estimatedCurrentTime, estimatedCurrentTime / 1000);
        Serial.printf("🕐 Écoulé depuis sync: %lu ms (%.1f minutes)\n", 
                     elapsedSinceStart, elapsedSinceStart / 60000.0);
    }
    
    // Debug timestamp session
    if (currentSessionStartTimestamp > 0) {
        Serial.printf("🕐 Timestamp session courante: %llu ms\n", currentSessionStartTimestamp);
    }
}