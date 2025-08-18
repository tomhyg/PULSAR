// battery_manager.cpp - Implémentation du gestionnaire de batterie (version corrigée)
#include "battery_manager.h"
#include "led_manager.h"

// Variables statiques
SFE_MAX1704X BatteryManager::fuelGauge;
bool BatteryManager::initialized = false;
bool BatteryManager::lowBatteryAlertEnabled = false;
float BatteryManager::lowBatteryThreshold = 15.0;
unsigned long BatteryManager::lastBatteryCheck = 0;
BatteryInfo BatteryManager::lastBatteryInfo = {0, 0, 0, false, false, false, "Unknown", false};
BatteryState BatteryManager::currentState = BATTERY_ERROR;

bool BatteryManager::init() {
    Serial.println("🔋 === INITIALISATION GESTIONNAIRE BATTERIE ===");
    
    if (!fuelGauge.begin()) {
        Serial.println("❌ MAX1704X non trouvé - Vérifier câblage");
        initialized = false;
        return false;
    }
    
    Serial.println("✅ MAX1704X détecté");
    
    // Configuration initiale
    fuelGauge.quickStart();
    delay(200);
    
    // Test de lecture
    float voltage = fuelGauge.getVoltage();
    float percentage = fuelGauge.getSOC();
    
    if (voltage < 2.0 || voltage > 6.0) {  // 🔧 Plage élargie pour USB
        Serial.printf("⚠️ Tension suspecte: %.2f V\n", voltage);
    }
    
    if (percentage > 150.0) {  // 🔧 Seuil plus tolérant
        Serial.printf("⚠️ Pourcentage suspect: %.1f%%\n", percentage);
    }
    
    initialized = true;
    Serial.printf("✅ Batterie initialisée: %.2f V, %.1f%%\n", voltage, percentage);
    
    // Mise à jour initiale
    updateBatteryInfo();
    
    return true;
}

BatteryInfo BatteryManager::getBatteryInfo() {
    if (!initialized) {
        return {0, 0, 0, false, false, false, "Capteur non initialisé", false};
    }
    
    // Mise à jour périodique (toutes les 10 secondes)
    unsigned long currentTime = millis();
    if (currentTime - lastBatteryCheck > 10000) {
        updateBatteryInfo();
        lastBatteryCheck = currentTime;
    }
    
    return lastBatteryInfo;
}

void BatteryManager::updateBatteryInfo() {
    if (!initialized) return;
    
    BatteryInfo info;
    
    // Lecture des données
    info.voltage = fuelGauge.getVoltage();
    info.percentage = fuelGauge.getSOC();
    info.changeRate = fuelGauge.getChangeRate();
    
    // 🔧 VALIDATION CORRIGÉE: Plus tolérante + correction automatique
    if (info.voltage < 2.0 || info.voltage > 6.0) {  // Plage élargie pour USB
        info.isValid = false;
        info.status = "Tension invalide";
        Serial.printf("⚠️ Données batterie invalides: tension %.2f V hors limites\n", info.voltage);
        return;
    }
    
    // 🔧 CORRECTION AUTOMATIQUE DU POURCENTAGE
    if (info.percentage > 100.0) {
        Serial.printf("🔧 Correction pourcentage: %.1f%% → 100.0%%\n", info.percentage);
        info.percentage = 100.0;  // Plafonner à 100%
    }
    if (info.percentage < 0.0) {
        Serial.printf("🔧 Correction pourcentage: %.1f%% → 0.0%%\n", info.percentage);
        info.percentage = 0.0;    // Plancher à 0%
    }
    
    // 🔧 TOUJOURS ACCEPTER LES DONNÉES CORRIGÉES
    info.isValid = true;
    
    // Détection de charge (taux de changement positif OU tension > 4.0V)
    info.isCharging = (info.changeRate > 0.1) || (info.voltage > 4.3);  // 🔧 Détection améliorée
    
    // Seuils de batterie (basés sur le pourcentage corrigé)
    info.isLowBattery = info.percentage <= 30.0;
    info.isCriticalBattery = info.percentage <= 15.0;
    
    // Calcul de l'état
    currentState = calculateBatteryState(info.percentage, info.changeRate);
    info.status = stateToString(currentState);
    
    lastBatteryInfo = info;
    
    // 🔧 LOG PLUS INFORMATIF
    Serial.printf("🔋 Batterie: %.2f V, %.1f%%, %s%s\n", 
                 info.voltage, info.percentage, info.status.c_str(),
                 info.isCharging ? " (charge)" : "");
}

BatteryState BatteryManager::calculateBatteryState(float percentage, float changeRate) {
    if (changeRate > 0.1) {
        return BATTERY_CHARGING;
    } else if (percentage <= 15.0) {
        return BATTERY_CRITICAL;
    } else if (percentage <= 30.0) {
        return BATTERY_LOW;
    } else {
        return BATTERY_NORMAL;
    }
}

String BatteryManager::stateToString(BatteryState state) {
    switch (state) {
        case BATTERY_NORMAL:    return "Normal";
        case BATTERY_LOW:       return "Faible";
        case BATTERY_CRITICAL:  return "Critique";
        case BATTERY_CHARGING:  return "En charge";
        case BATTERY_ERROR:     return "Erreur";
        default:                return "Inconnu";
    }
}

BatteryState BatteryManager::getBatteryState() {
    getBatteryInfo(); // Force mise à jour
    return currentState;
}

float BatteryManager::getVoltage() {
    if (!initialized) return 0.0;
    return fuelGauge.getVoltage();
}

float BatteryManager::getPercentage() {
    if (!initialized) return 0.0;
    
    // 🔧 RETOURNER POURCENTAGE CORRIGÉ
    float percentage = fuelGauge.getSOC();
    if (percentage > 100.0) return 100.0;
    if (percentage < 0.0) return 0.0;
    return percentage;
}

float BatteryManager::getChangeRate() {
    if (!initialized) return 0.0;
    return fuelGauge.getChangeRate();
}

bool BatteryManager::isCharging() {
    if (!initialized) return false;
    
    // 🔧 DÉTECTION DE CHARGE AMÉLIORÉE
    float voltage = fuelGauge.getVoltage();
    float changeRate = fuelGauge.getChangeRate();
    return (changeRate > 0.1) || (voltage > 4.3);  // Charge si taux positif OU tension élevée
}

String BatteryManager::getBatteryStatusText() {
    BatteryInfo info = getBatteryInfo();
    if (!info.isValid) return "Capteur indisponible";
    
    String status = String(info.percentage, 1) + "% ";
    status += "(" + String(info.voltage, 2) + "V) ";
    status += info.status;
    
    if (info.isCharging) {
        status += " 🔌";
    } else if (info.isCriticalBattery) {
        status += " ⚠️";
    } else if (info.isLowBattery) {
        status += " 🔋";
    }
    
    return status;
}

void BatteryManager::printBatteryInfo() {
    BatteryInfo info = getBatteryInfo();
    
    Serial.println("🔋 === INFORMATIONS BATTERIE ===");
    if (!info.isValid) {
        Serial.println("❌ Données batterie invalides");
        return;
    }
    
    Serial.printf("   🔋 Tension: %.2f V\n", info.voltage);
    Serial.printf("   📊 Charge: %.1f%%\n", info.percentage);
    Serial.printf("   📈 Taux de changement: %.2f%%/h\n", info.changeRate);
    Serial.printf("   🔌 En charge: %s\n", info.isCharging ? "OUI" : "NON");
    Serial.printf("   ⚡ État: %s\n", info.status.c_str());
    
    if (info.isLowBattery) {
        Serial.println("   ⚠️ ATTENTION: Batterie faible!");
    }
    if (info.isCriticalBattery) {
        Serial.println("   🚨 CRITIQUE: Batterie très faible!");
    }
    
    Serial.println("🔋 ===============================");
}

void BatteryManager::enableLowBatteryAlert(float threshold) {
    lowBatteryAlertEnabled = true;
    lowBatteryThreshold = threshold;
    Serial.printf("🔋 Alerte batterie faible activée: %.1f%%\n", threshold);
}

void BatteryManager::disableLowBatteryAlert() {
    lowBatteryAlertEnabled = false;
    Serial.println("🔋 Alerte batterie faible désactivée");
}

bool BatteryManager::isLowBatteryAlertEnabled() {
    return lowBatteryAlertEnabled;
}

void BatteryManager::checkBatteryAlert() {
    if (!lowBatteryAlertEnabled || !initialized) return;
    
    BatteryInfo info = getBatteryInfo();
    if (!info.isValid) return;
    
    static bool alertActive = false;
    static unsigned long lastAlert = 0;
    
    if (info.percentage <= lowBatteryThreshold && !info.isCharging) {
        if (!alertActive || (millis() - lastAlert > 60000)) { // Alerte toutes les minutes
            Serial.printf("🚨 ALERTE BATTERIE: %.1f%% (seuil: %.1f%%)\n", 
                         info.percentage, lowBatteryThreshold);
            
            // Clignotement rouge pour alerte batterie
            LEDManager::setColor(255, 0, 0);
            delay(500);
            LEDManager::setColor(0, 0, 0);
            delay(500);
            LEDManager::setColor(255, 0, 0);
            delay(500);
            LEDManager::setColor(0, 0, 0);
            
            alertActive = true;
            lastAlert = millis();
        }
    } else {
        alertActive = false;
    }
}

void BatteryManager::sleep() {
    if (!initialized) return;
    // Le MAX1704x n'a pas de fonction sleep explicite
    // Il est déjà très basse consommation
    Serial.println("🔋 Fuel gauge en mode basse consommation");
}

void BatteryManager::wake() {
    if (!initialized) return;
    // Réveil automatique, pas d'action nécessaire
    Serial.println("🔋 Fuel gauge réveillé");
}

void BatteryManager::quickStart() {
    if (!initialized) return;
    fuelGauge.quickStart();
    Serial.println("🔋 QuickStart fuel gauge exécuté");
}

String BatteryManager::getBatteryDataForAWS() {
    BatteryInfo info = getBatteryInfo();
    if (!info.isValid) {
        return "\"battery_voltage\": \"0.00\", \"battery_percentage\": \"0.0\", \"battery_status\": \"error\"";
    }
    
    String data = "\"battery_voltage\": \"" + String(info.voltage, 2) + "\",";
    data += "\"battery_percentage\": \"" + String(info.percentage, 1) + "\",";
    data += "\"battery_change_rate\": \"" + String(info.changeRate, 2) + "\",";
    data += "\"battery_charging\": \"" + String(info.isCharging ? "true" : "false") + "\",";
    data += "\"battery_status\": \"" + info.status + "\"";
    
    return data;
}

void BatteryManager::addBatteryDataToPayload(String& payload) {
    // Remplacer l'ancienne ligne battery_level par les nouvelles données
    String batteryData = getBatteryDataForAWS();
    
    // Chercher et remplacer "battery_level": "3.88"
    int start = payload.indexOf("\"battery_level\"");
    if (start != -1) {
        int end = payload.indexOf(",", start);
        if (end != -1) {
            // Remplacer la ligne complète
            payload = payload.substring(0, start) + batteryData + payload.substring(end);
        }
    } else {
        // Ajouter à la fin avant la fermeture
        int lastComma = payload.lastIndexOf(",");
        if (lastComma != -1) {
            payload = payload.substring(0, lastComma + 1) + batteryData + "," + payload.substring(lastComma + 1);
        }
    }
}