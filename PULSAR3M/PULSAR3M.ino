// main.ino - Version simplifiée avec intégration batterie basique
#include "config.h"
#include "led_manager.h"
#include "spi_manager.h"
#include "sensor_manager.h"
#include "web_server_manager.h"
#include "data_manager.h"
#include "system_manager.h"
#include "secrets_aws_rest.h"
#include "frequency_diagnostic.h"
#include "watch_removal_detector.h"
#include "battery_manager.h"  // 🔋 Gestionnaire batterie

// Variables pour debug crash
unsigned long lastLoopTime = 0;
unsigned long loopCounter = 0;

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("🔧 Configuration FreeRTOS...");
  
  // DEBUG PSRAM
  Serial.println("=== DEBUG MÉMOIRE ===");
  Serial.printf("PSRAM trouvée: %s\n", psramFound() ? "OUI" : "NON");
  if (psramFound()) {
      Serial.printf("Taille PSRAM: %d bytes\n", ESP.getPsramSize());
      Serial.printf("PSRAM libre: %d bytes\n", ESP.getFreePsram());
  }
  Serial.printf("RAM interne libre: %d bytes\n", ESP.getFreeHeap());
  Serial.println("======================");
  
  Serial.println("\n=== PULSAR-007 INIT UNIFIÉ AVEC BATTERIE SIMPLE ===");
  
  // 🔧 Test LED en premier pour voir si c'est la cause
  Serial.println("🔧 Test LED avant autres initialisations...");
  LEDManager::init();
  LEDManager::setState(LED_CONFIG_AP);
  delay(1000);
  
  Serial.println("🔧 LED OK, continuation...");
  
  // Initialisation des modules
  SystemManager::loadConfiguration();
  SPIManager::initializeConfigMode();
  
  // Initialisation I2C
  Wire.begin(7, 6, 400000);
  
  // 🔋 NOUVEAU: Initialiser fuel gauge EN PREMIER (avant d'éteindre le PPG)
  Serial.println("🔋 Initialisation gestionnaire batterie...");
  if (BatteryManager::init()) {
    Serial.println("✅ Gestionnaire batterie initialisé");
    BatteryManager::printBatteryInfo();
  } else {
    Serial.println("⚠️ Gestionnaire batterie non disponible");
  }
  
  // Initialisation PPG (après fuel gauge)
  if (!SensorManager::initPPGSensor()) {
    Serial.println("❌ MAX86916 non trouvé");
    LEDManager::setState(LED_ERROR);
    while (1) { 
      LEDManager::update(); 
      delay(1000);
    }
  }
  
  // 🔋 Éteindre le PPG APRÈS initialisation fuel gauge
  Serial.println("🔋 ARRÊT PPG pendant mode configuration (fuel gauge déjà initialisé)");
  SensorManager::powerDownPPG();
  
  // Configuration WiFi AVANT de démarrer l'AP
  Serial.println("🌐 Configuration WiFi...");
  WiFi.mode(WIFI_OFF);
  delay(1000);
  
  // Point d'accès WiFi avec configuration spécifique
  WiFi.mode(WIFI_MODE_AP);
  delay(500);
  
  // Configuration AP avec paramètres optimisés
  WiFi.softAPConfig(
    IPAddress(192, 168, 4, 1),    // IP
    IPAddress(192, 168, 4, 1),    // Gateway  
    IPAddress(255, 255, 255, 0)   // Subnet
  );
  
  if (WiFi.softAP("Pulsar-007", "12345678", 1, 0, 4)) {
    Serial.println("✅ Point d'accès démarré:");
    Serial.println("   SSID: Pulsar-007");
    Serial.println("   Password: 12345678");
    Serial.println("   IP: " + WiFi.softAPIP().toString());
    Serial.println("   Ouvrez http://" + WiFi.softAPIP().toString() + " dans votre navigateur");
    Serial.println("🔍 Système de détection de retrait intégré");
    Serial.println("🔋 Monitoring batterie simple intégré");
  } else {
    Serial.println("❌ Erreur démarrage point d'accès");
    LEDManager::setState(LED_ERROR);
    while (1) delay(1000);
  }
  
  WebServerManager::setup();
  
  Serial.println("🎯 Système en attente de configuration...");
  
  // 🔋 Affichage état batterie initial
  BatteryManager::printBatteryInfo();
}

void loop() {
    unsigned long currentTime = millis();
    unsigned long currentMicros = micros();
    
    // 🔧 Debug pour détecter les boucles problématiques
    loopCounter++;
    if (currentTime - lastLoopTime > 5000) {  // Toutes les 5 secondes
        Serial.printf("🔧 DEBUG: Loop OK - %lu iterations en 5s\n", loopCounter);
        Serial.printf("🔧 Mode: %d, RAM libre: %d KB\n", 
                     SystemManager::getCurrentMode(), ESP.getFreeHeap() / 1024);
        
        // 🔋 Debug batterie périodique (simple)
        BatteryInfo batteryInfo = BatteryManager::getBatteryInfo();
        if (batteryInfo.isValid) {
            Serial.printf("🔋 Batterie: %.1f%% (%.2fV) - %s\n",
                         batteryInfo.percentage, batteryInfo.voltage, batteryInfo.status.c_str());
        }
        
        // 🔧 Debug diagnostic unifié
        if (isSessionDiagnosticActive()) {
            SessionStats stats = getSessionStats();
            Serial.printf("📊 Session #%d active: %lu échantillons, %.1f Hz, %.1f%% efficacité\n",
                         stats.sessionNumber, stats.sampleCount, 
                         stats.currentFrequency, stats.efficiency);
        }
        
        // 🔍 Debug détecteur de retrait
        if (WatchRemovalDetector::isEnabled()) {
            DetectorState detectorState = WatchRemovalDetector::getState();
            Serial.printf("🔍 Détecteur retrait: %s\n",
                         (detectorState == DETECTOR_NORMAL) ? "NORMAL" :
                         (detectorState == DETECTOR_SUSPICIOUS) ? "SUSPECT" :
                         (detectorState == DETECTOR_REMOVED) ? "RETIRÉE" : "REDÉMARRAGE");
        }
        
        lastLoopTime = currentTime;
        loopCounter = 0;
    }
    
    // Mode configuration
    if (SystemManager::getCurrentMode() == MODE_CONFIG) {
        // 🔧 SIMPLIFICATION: Réduire la fréquence des appels
        static unsigned long lastWebHandling = 0;
        if (currentTime - lastWebHandling > 10) {  // Toutes les 10ms au lieu de chaque loop
            WebServerManager::handleClient();
            lastWebHandling = currentTime;
        }
        
        // 🔧 SIMPLIFICATION: LED update moins fréquent
        static unsigned long lastLEDUpdate = 0;
        if (currentTime - lastLEDUpdate > 500) {  // Toutes les 500ms
            LEDManager::update();
            lastLEDUpdate = currentTime;
        }
        
        // Vérification périodique de l'état WiFi AP
        static unsigned long lastAPCheck = 0;
        if (currentTime - lastAPCheck > 30000) { // Toutes les 30 secondes
            lastAPCheck = currentTime;
            Serial.printf("🔧 AP Status: Mode=%d, Stations=%d\n", 
                         WiFi.getMode(), WiFi.softAPgetStationNum());
        }
        
        // 🔧 IMPORTANT: Délai pour éviter surcharge
        delay(10);
        return;
    }
    
    // 🔧 Mode acquisition unifié
    if (!SPIManager::isSharedMode()) {
        Serial.println("⚠️ Mode acquisition mais SPI pas en mode partagé");
        delay(1000);
        return;
    }

    // 🔧 Démarrage automatique du diagnostic à la première acquisition
    static bool firstAcquisition = true;
    if (firstAcquisition && SensorManager::isPPGPowered() && bufferIndex == 0) {
        Serial.println("🚀 Démarrage première acquisition avec diagnostic unifié");
        startSessionDiagnostic(sessionCounter + 1);
        
        // 🔍 Activer le détecteur de retrait de montre
        WatchRemovalDetector::setEnabled(true);
        Serial.println("🔍 Détecteur de retrait activé");
        
        firstAcquisition = false;
    }

    // 🔍 Mise à jour du détecteur de retrait de montre
    WatchRemovalDetector::update();
    
    // 🔍 Vérifier si la montre a été retirée - VERSION SIMPLIFIÉE
    if (WatchRemovalDetector::isWatchRemoved()) {
        Serial.println("🔍 ❌ === MONTRE RETIRÉE DÉTECTÉE ===");
        
        // Arrêter le diagnostic en cours
        if (isSessionDiagnosticActive()) {
            Serial.println("🔍 Arrêt du diagnostic en cours");
            stopSessionDiagnostic();
        }
        
        // Arrêter l'acquisition
        Serial.println("🔍 Arrêt de l'acquisition");
        SensorManager::powerDownPPG();
        
        // LED rouge clignotante
        LEDManager::setState(LED_ERROR);
        
        // Attendre 5 secondes avec clignotement
        Serial.println("🔍 ⏰ Attente 5 secondes avant redémarrage...");
        for (int i = 0; i < 10; i++) {
            LEDManager::setColor(255, 0, 0);  // Rouge
            delay(250);
            LEDManager::setColor(0, 0, 0);    // Éteint
            delay(250);
            Serial.printf("🔍 Redémarrage dans %d secondes...\n", 5 - (i/2));
        }
        
        // Redémarrage immédiat
        Serial.println("🔍 🔄 === REDÉMARRAGE IMMÉDIAT ===");
        Serial.flush();
        delay(500);
        ESP.restart();
    }

    // Mise à jour des capteurs
    SensorManager::updatePPGSensor();
    
    // 🔧 Acquisition des données (avec comptage automatique via countSample())
    DataManager::acquireData(currentMicros);
    
    // 🔧 Traitement du buffer (avec gestion automatique du diagnostic)
    DataManager::processBuffer();
    
    // 🔧 CORRECTION: LED update moins fréquent en mode acquisition
    static unsigned long lastLEDUpdateAcq = 0;
    if (currentTime - lastLEDUpdateAcq > 1000) {  // Toutes les secondes
        LEDManager::update();
        lastLEDUpdateAcq = currentTime;
    }
    
    // Debug système périodique avec info diagnostic et batterie
    static unsigned long lastDebug = 0;
    if (currentTime - lastDebug > 60000) {  // Toutes les minutes
        lastDebug = currentTime;
        Serial.printf("🔧 DEBUG SYSTÈME: Session #%d, PPG: %s, Buffer: %d/%d\n",
                     sessionCounter + 1, 
                     SensorManager::isPPGPowered() ? "ON" : "OFF",
                     bufferIndex, currentBufferSize);
        
        // 🔋 Affichage batterie simple
        BatteryManager::printBatteryInfo();
        
        // 🔧 Affichage stats diagnostic
        if (isSessionDiagnosticActive()) {
            SessionStats stats = getSessionStats();
            Serial.printf("📊 Diagnostic actif: Session #%d, %.1f Hz, %.1f%% efficacité\n",
                         stats.sessionNumber, stats.currentFrequency, stats.efficiency);
        }
        
        // 🔍 Diagnostic du détecteur de retrait
        if (WatchRemovalDetector::isEnabled()) {
            SignalStats removalStats = WatchRemovalDetector::getLastStats();
            DetectorState detectorState = WatchRemovalDetector::getState();
            Serial.printf("🔍 Détecteur retrait: %s, Amplitude: %u, Moyenne: %u\n",
                         (detectorState == DETECTOR_NORMAL) ? "NORMAL" :
                         (detectorState == DETECTOR_SUSPICIOUS) ? "SUSPECT" :
                         (detectorState == DETECTOR_REMOVED) ? "RETIRÉE" : "REDÉMARRAGE",
                         removalStats.amplitude, removalStats.mean);
        }
        
        SystemManager::printDebugInfo();
    }
    
    // 🔧 CORRECTION: Délai minimal plus long
    delayMicroseconds(100);
}