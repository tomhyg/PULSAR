// watch_removal_detector.cpp - Implémentation du détecteur de retrait (SANS VARIANCE)
#include "watch_removal_detector.h"
#include "led_manager.h"
#include "system_manager.h"
#include "sensor_manager.h"
#include "data_manager.h"
#include "frequency_diagnostic.h"

// Variables statiques
uint32_t WatchRemovalDetector::ppgBuffer[DETECTOR_WINDOW_SIZE];
int WatchRemovalDetector::bufferIndex = 0;
int WatchRemovalDetector::sampleCount = 0;
unsigned long WatchRemovalDetector::lastCheckTime = 0;
unsigned long WatchRemovalDetector::removalDetectedTime = 0;
DetectorState WatchRemovalDetector::currentState = DETECTOR_NORMAL;
bool WatchRemovalDetector::enabled = false;
SignalStats WatchRemovalDetector::lastStats = {0, 0, 0, 0, false};  // SANS VARIANCE
int WatchRemovalDetector::consecutiveAnomalies = 0;
bool WatchRemovalDetector::bufferFull = false;

// Seuils ajustables (SANS VARIANCE)
uint32_t WatchRemovalDetector::amplitudeThresholdLow = AMPLITUDE_THRESHOLD_LOW;
uint32_t WatchRemovalDetector::amplitudeThresholdHigh = AMPLITUDE_THRESHOLD_HIGH;
uint32_t WatchRemovalDetector::meanThresholdLow = MEAN_THRESHOLD_LOW;
uint32_t WatchRemovalDetector::meanThresholdHigh = MEAN_THRESHOLD_HIGH;

void WatchRemovalDetector::init() {
    Serial.println("🔍 === INITIALISATION DÉTECTEUR RETRAIT MONTRE ===");
    
    // Initialiser le buffer
    memset(ppgBuffer, 0, sizeof(ppgBuffer));
    bufferIndex = 0;
    sampleCount = 0;
    bufferFull = false;
    
    // État initial
    currentState = DETECTOR_NORMAL;
    consecutiveAnomalies = 0;
    lastCheckTime = millis();
    removalDetectedTime = 0;
    
    // Désactivé par défaut, sera activé lors du démarrage de l'acquisition
    enabled = false;
    
    Serial.println("🔍 Configuration seuils:");
    Serial.printf("   📊 Amplitude: %u - %u\n", amplitudeThresholdLow, amplitudeThresholdHigh);
    Serial.printf("   📊 Moyenne: %u - %u\n", meanThresholdLow, meanThresholdHigh);
    Serial.printf("   ⏱️ Fenêtre: %d échantillons (%.1f sec)\n", 
                 DETECTOR_WINDOW_SIZE, DETECTOR_WINDOW_SIZE / 100.0);
    
    Serial.println("✅ Détecteur initialisé (désactivé)");
}

void WatchRemovalDetector::setEnabled(bool enable) {
    if (enable && !enabled) {
        Serial.println("🔍 ACTIVATION détecteur retrait montre");
        enabled = true;
        reset();
    } else if (!enable && enabled) {
        Serial.println("🔍 DÉSACTIVATION détecteur retrait montre");
        enabled = false;
        currentState = DETECTOR_NORMAL;
    }
}

bool WatchRemovalDetector::isEnabled() {
    return enabled;
}

void WatchRemovalDetector::reset() {
    Serial.println("🔍 RESET détecteur retrait montre");
    bufferIndex = 0;
    sampleCount = 0;
    bufferFull = false;
    consecutiveAnomalies = 0;
    currentState = DETECTOR_NORMAL;
    removalDetectedTime = 0;
    lastCheckTime = millis();
    memset(ppgBuffer, 0, sizeof(ppgBuffer));
}

void WatchRemovalDetector::addPPGSample(uint32_t green, uint32_t ir, uint32_t red) {
    if (!enabled || currentState == DETECTOR_RESTARTING) {
        return;
    }
    
    // Utiliser le canal GREEN comme référence principal
    uint32_t ppgValue = green;
    
    // Ajouter au buffer circulaire
    ppgBuffer[bufferIndex] = ppgValue;
    bufferIndex = (bufferIndex + 1) % DETECTOR_WINDOW_SIZE;
    
    if (sampleCount < DETECTOR_WINDOW_SIZE) {
        sampleCount++;
    } else {
        bufferFull = true;
    }
}

void WatchRemovalDetector::update() {
    if (!enabled || currentState == DETECTOR_RESTARTING) {
        return;
    }
    
    unsigned long currentTime = millis();
    
    // Vérification périodique
    if (currentTime - lastCheckTime < DETECTOR_CHECK_INTERVAL) {
        return;
    }
    
    lastCheckTime = currentTime;
    
    // Besoin d'au moins 2 secondes de données (200 échantillons)
    if (sampleCount < 200) {
        return;
    }
    
    // Calculer les statistiques
    SignalStats stats = calculateStats();
    lastStats = stats;
    
    if (!stats.isValid) {
        Serial.println("🔍 ⚠️ Erreur calcul statistiques PPG");
        return;
    }
    
    // Vérifier si le signal est anormal
    bool isAnomalous = isSignalAnomalous(stats);
    
    if (isAnomalous) {
        consecutiveAnomalies++;
        Serial.printf("🔍 ⚠️ Signal anormal détecté (%d/3)\n", consecutiveAnomalies);
        Serial.printf("   📊 Amplitude: %u, Moyenne: %u\n", 
                     stats.amplitude, stats.mean);
        
        if (consecutiveAnomalies >= 5) {
            Serial.println("🔍 ❌ MONTRE RETIRÉE DÉTECTÉE!");
            handleRemovalDetected();
        } else {
            currentState = DETECTOR_SUSPICIOUS;
        }
    } else {
        // Signal normal, réinitialiser le compteur
        if (consecutiveAnomalies > 0) {
            Serial.println("🔍 ✅ Signal redevenu normal");
            consecutiveAnomalies = 0;
        }
        currentState = DETECTOR_NORMAL;
    }
}

SignalStats WatchRemovalDetector::calculateStats() {
    SignalStats stats = {0, 0, 0, 0, false};  // SANS VARIANCE
    
    if (sampleCount < 10) {
        return stats;
    }
    
    // Calculer min, max et moyenne
    uint32_t minVal = 0xFFFFFFFF;
    uint32_t maxVal = 0;
    uint64_t sum = 0;
    
    int samplesToAnalyze = min(sampleCount, DETECTOR_WINDOW_SIZE);
    
    for (int i = 0; i < samplesToAnalyze; i++) {
        uint32_t value = ppgBuffer[i];
        
        if (value < minVal) minVal = value;
        if (value > maxVal) maxVal = value;
        sum += value;
    }
    
    stats.minValue = minVal;
    stats.maxValue = maxVal;
    stats.amplitude = maxVal - minVal;
    stats.mean = (uint32_t)(sum / samplesToAnalyze);
    
    // PAS DE CALCUL DE VARIANCE !
    
    stats.isValid = true;
    return stats;
}

bool WatchRemovalDetector::isSignalAnomalous(const SignalStats& stats) {
    bool anomalous = false;
    
    // Vérifier l'amplitude (trop faible ou trop forte)
    if (stats.amplitude < amplitudeThresholdLow) {
        Serial.printf("🔍 Amplitude trop faible: %u < %u\n", stats.amplitude, amplitudeThresholdLow);
        anomalous = true;
    } else if (stats.amplitude > amplitudeThresholdHigh) {
        Serial.printf("🔍 Amplitude trop forte: %u > %u\n", stats.amplitude, amplitudeThresholdHigh);
        anomalous = true;
    }
    
    // Vérifier la moyenne (signal trop faible ou saturé)
    if (stats.mean < meanThresholdLow) {
        Serial.printf("🔍 Moyenne trop faible: %u < %u\n", stats.mean, meanThresholdLow);
        anomalous = true;
    } else if (stats.mean > meanThresholdHigh) {
        Serial.printf("🔍 Moyenne trop forte: %u > %u\n", stats.mean, meanThresholdHigh);
        anomalous = true;
    }
    
    // PAS DE VÉRIFICATION DE VARIANCE !
    
    return anomalous;
}

void WatchRemovalDetector::handleRemovalDetected() {
    currentState = DETECTOR_REMOVED;
    removalDetectedTime = millis();
    
    Serial.println("🔍 ❌ === MONTRE RETIRÉE DÉTECTÉE ===");
    Serial.println("🔍 📊 Statistiques au moment de la détection:");
    Serial.printf("   📊 Amplitude: %u (seuil: %u-%u)\n", 
                 lastStats.amplitude, amplitudeThresholdLow, amplitudeThresholdHigh);
    Serial.printf("   📊 Moyenne: %u (seuil: %u-%u)\n", 
                 lastStats.mean, meanThresholdLow, meanThresholdHigh);
    
    // Arrêter le diagnostic en cours
    if (isSessionDiagnosticActive()) {
        Serial.println("🔍 Arrêt du diagnostic en cours");
        stopSessionDiagnostic();
    }
    
    // Arrêter l'acquisition
    Serial.println("🔍 Arrêt de l'acquisition en cours");
    SensorManager::powerDownPPG();
    
    // Passer en mode LED d'erreur (rouge clignotant)
    LEDManager::setState(LED_ERROR);
    
    // 🔧 SIMPLIFICATION: Pas d'état RESTARTING, le main.ino gère directement
    Serial.println("🔍 ⚠️ Retrait confirmé - Redémarrage géré par main.ino");
}

void WatchRemovalDetector::performRestart() {
    Serial.println("🔍 🔄 === REDÉMARRAGE AUTOMATIQUE ===");
    
    // Attendre le délai de redémarrage
    unsigned long currentTime = millis();
    if (currentTime - removalDetectedTime < DETECTOR_RESTART_DELAY) {
        return;
    }
    
    Serial.println("🔍 🔄 Redémarrage du système...");
    
    // Réinitialiser le détecteur
    reset();
    
    // Redémarrer le système
    Serial.println("🔍 🔄 Redémarrage ESP32...");
    delay(1000);
    ESP.restart();
}

bool WatchRemovalDetector::isWatchRemoved() {
    return currentState == DETECTOR_REMOVED;  // 🔧 SIMPLIFICATION: Seulement REMOVED
}

DetectorState WatchRemovalDetector::getState() {
    return currentState;
}

SignalStats WatchRemovalDetector::getLastStats() {
    return lastStats;
}

void WatchRemovalDetector::printDiagnostics() {
    Serial.println("🔍 === DIAGNOSTIC DÉTECTEUR RETRAIT ===");
    Serial.printf("   🔧 État: %s\n", 
                 (currentState == DETECTOR_NORMAL) ? "NORMAL" :
                 (currentState == DETECTOR_SUSPICIOUS) ? "SUSPECT" :
                 (currentState == DETECTOR_REMOVED) ? "RETIRÉE" : "REDÉMARRAGE");
    Serial.printf("   🔧 Activé: %s\n", enabled ? "OUI" : "NON");
    Serial.printf("   📊 Échantillons: %d/%d\n", sampleCount, DETECTOR_WINDOW_SIZE);
    Serial.printf("   ⚠️ Anomalies consécutives: %d/3\n", consecutiveAnomalies);
    
    if (lastStats.isValid) {
        Serial.printf("   📊 Dernières stats:\n");
        Serial.printf("      Amplitude: %u (seuil: %u-%u)\n", 
                     lastStats.amplitude, amplitudeThresholdLow, amplitudeThresholdHigh);
        Serial.printf("      Moyenne: %u (seuil: %u-%u)\n", 
                     lastStats.mean, meanThresholdLow, meanThresholdHigh);
    }
    
    Serial.println("🔍 === FIN DIAGNOSTIC ===");
}

void WatchRemovalDetector::calibrateThresholds() {
    Serial.println("🔍 🔧 === CALIBRATION AUTOMATIQUE DES SEUILS ===");
    
    if (sampleCount < DETECTOR_WINDOW_SIZE) {
        Serial.println("🔍 ⚠️ Pas assez de données pour calibration");
        return;
    }
    
    SignalStats stats = calculateStats();
    if (!stats.isValid) {
        Serial.println("🔍 ❌ Erreur lors du calcul des statistiques");
        return;
    }
    
    Serial.println("🔍 📊 Statistiques actuelles:");
    Serial.printf("   Amplitude: %u\n", stats.amplitude);
    Serial.printf("   Moyenne: %u\n", stats.mean);
    
    // Ajuster les seuils basés sur les données actuelles
    amplitudeThresholdLow = stats.amplitude * 0.3;  // 30% de l'amplitude actuelle
    amplitudeThresholdHigh = stats.amplitude * 5.0; // 500% de l'amplitude actuelle
    meanThresholdLow = stats.mean * 0.5;             // 50% de la moyenne actuelle
    meanThresholdHigh = stats.mean * 2.0;            // 200% de la moyenne actuelle
    
    Serial.println("🔍 🔧 Nouveaux seuils calibrés:");
    Serial.printf("   Amplitude: %u - %u\n", amplitudeThresholdLow, amplitudeThresholdHigh);
    Serial.printf("   Moyenne: %u - %u\n", meanThresholdLow, meanThresholdHigh);
    
    Serial.println("🔍 ✅ Calibration terminée");
}

void WatchRemovalDetector::setCustomThresholds(uint32_t ampLow, uint32_t ampHigh, 
                                              uint32_t meanLow, uint32_t meanHigh) {
    amplitudeThresholdLow = ampLow;
    amplitudeThresholdHigh = ampHigh;
    meanThresholdLow = meanLow;
    meanThresholdHigh = meanHigh;
    
    Serial.println("🔍 🔧 Seuils personnalisés définis:");
    Serial.printf("   Amplitude: %u - %u\n", amplitudeThresholdLow, amplitudeThresholdHigh);
    Serial.printf("   Moyenne: %u - %u\n", meanThresholdLow, meanThresholdHigh);
}