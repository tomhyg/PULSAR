// frequency_diagnostic.cpp - Système de diagnostic de fréquence unifié
#include "frequency_diagnostic.h"
#include <Arduino.h>

// Variables internes du diagnostic
static struct {
    bool active;                    // Diagnostic actif ou non
    unsigned long startTime;        // Début de session (millis)
    unsigned long sampleCount;      // Compteur d'échantillons
    unsigned long lastReport;       // Dernier rapport (millis)
    int sessionNumber;              // Numéro de session courante
    unsigned long targetSamples;    // Nombre d'échantillons cible
} diagnostic = {false, 0, 0, 0, 0, 30000};

// Démarrer le diagnostic pour une nouvelle session
void startSessionDiagnostic(int sessionNum) {
    diagnostic.active = true;
    diagnostic.startTime = millis();
    diagnostic.sampleCount = 0;
    diagnostic.lastReport = diagnostic.startTime;
    diagnostic.sessionNumber = sessionNum;
    
    Serial.printf("📊 === DÉBUT DIAGNOSTIC FRÉQUENCE SESSION #%d ===\n", sessionNum);
    Serial.printf("📊 Cible: %lu échantillons à 100 Hz\n", diagnostic.targetSamples);
    Serial.printf("📊 Début: %lu ms\n", diagnostic.startTime);
    Serial.println("📊 " + String('=', 50));
}

// Compter un échantillon (appelé dans acquireData)
void countSample() {
    if (!diagnostic.active) return;
    
    diagnostic.sampleCount++;
    
    // Rapport périodique toutes les 10 secondes
    unsigned long currentTime = millis();
    if (currentTime - diagnostic.lastReport >= 10000) {
        reportSessionProgress();
        diagnostic.lastReport = currentTime;
    }
}

// Rapport de progression toutes les 10 secondes
void reportSessionProgress() {
    if (!diagnostic.active) return;
    
    unsigned long elapsed = millis() - diagnostic.startTime;
    if (elapsed == 0) return; // Éviter division par zéro
    
    // Calculs de fréquence
    float sessionFrequency = (float)diagnostic.sampleCount * 1000.0 / elapsed;
    float efficiency = (sessionFrequency / 100.0) * 100.0;
    float progress = (float)diagnostic.sampleCount * 100.0 / diagnostic.targetSamples;
    
    // Estimation temps restant
    float estimatedTotalTime = (float)diagnostic.targetSamples * elapsed / diagnostic.sampleCount;
    float timeRemaining = estimatedTotalTime - elapsed;
    
    Serial.printf("📊 === SESSION #%d - PROGRESSION (%.1f sec) ===\n", 
                 diagnostic.sessionNumber, elapsed / 1000.0);
    Serial.printf("📊 Échantillons: %lu/%lu (%.1f%%)\n", 
                 diagnostic.sampleCount, diagnostic.targetSamples, progress);
    Serial.printf("📊 Fréquence actuelle: %.2f Hz (cible: 100 Hz)\n", sessionFrequency);
    Serial.printf("📊 Efficacité: %.1f%%\n", efficiency);
    Serial.printf("📊 Temps restant estimé: %.1f sec\n", timeRemaining / 1000.0);
    
    // Alertes qualité
    if (efficiency < 90.0) {
        Serial.println("⚠️ ATTENTION: Fréquence basse (<90%) - Vérifier système");
    } else if (efficiency > 110.0) {
        Serial.println("⚠️ ATTENTION: Fréquence élevée (>110%) - Possible problème timing");
    } else {
        Serial.println("✅ Fréquence dans la plage acceptable (90-110%)");
    }
    
    Serial.println("📊 " + String('-', 50));
}

// Arrêter le diagnostic et afficher le rapport final
void stopSessionDiagnostic() {
    if (!diagnostic.active) {
        Serial.println("📊 Diagnostic déjà arrêté");
        return;
    }
    
    unsigned long totalTime = millis() - diagnostic.startTime;
    
    // Rapport final complet
    Serial.println("📊 " + String('=', 60));
    Serial.printf("📊 === RAPPORT FINAL SESSION #%d ===\n", diagnostic.sessionNumber);
    Serial.println("📊 " + String('=', 60));
    
    // Données brutes
    Serial.printf("📊 Durée totale: %lu ms (%.1f sec, %.1f min)\n", 
                 totalTime, totalTime / 1000.0, totalTime / 60000.0);
    Serial.printf("📊 Échantillons acquis: %lu\n", diagnostic.sampleCount);
    Serial.printf("📊 Échantillons cible: %lu\n", diagnostic.targetSamples);
    
    // Calculs de performance
    if (totalTime > 0) {
        float averageFrequency = (float)diagnostic.sampleCount * 1000.0 / totalTime;
        float globalEfficiency = (averageFrequency / 100.0) * 100.0;
        unsigned long expectedSamples = totalTime / 10; // 100Hz = 10ms par échantillon
        long missedSamples = expectedSamples - diagnostic.sampleCount;
        
        Serial.printf("📊 Fréquence moyenne: %.2f Hz\n", averageFrequency);
        Serial.printf("📊 Efficacité globale: %.1f%%\n", globalEfficiency);
        Serial.printf("📊 Échantillons prévus: %lu\n", expectedSamples);
        Serial.printf("📊 Échantillons manqués: %ld\n", missedSamples);
        
        // Évaluation qualité
        Serial.println("📊 " + String('-', 40));
        if (globalEfficiency < 85.0) {
            Serial.println("❌ QUALITÉ: TRÈS INSUFFISANTE (<85%)");
            Serial.println("   → Vérifier capteurs et timing système");
        } else if (globalEfficiency < 95.0) {
            Serial.println("⚠️ QUALITÉ: INSUFFISANTE (85-95%)");
            Serial.println("   → Optimisations possibles nécessaires");
        } else if (globalEfficiency > 115.0) {
            Serial.println("⚠️ QUALITÉ: SURÉCHANTILLONNAGE (>115%)");
            Serial.println("   → Vérifier logique de timing");
        } else {
            Serial.println("✅ QUALITÉ: EXCELLENTE (95-115%)");
            Serial.println("   → Acquisition optimale");
        }
    } else {
        Serial.println("❌ ERREUR: Durée nulle, impossible de calculer la fréquence");
    }
    
    Serial.println("📊 " + String('=', 60));
    Serial.println("📊 === FIN DIAGNOSTIC SESSION ===");
    Serial.println("📊 " + String('=', 60));
    
    // Réinitialiser le diagnostic
    diagnostic.active = false;
    diagnostic.startTime = 0;
    diagnostic.sampleCount = 0;
    diagnostic.lastReport = 0;
    diagnostic.sessionNumber = 0;
}

// Vérifier si le diagnostic est actif
bool isSessionDiagnosticActive() {
    return diagnostic.active;
}

// Obtenir les statistiques actuelles
SessionStats getSessionStats() {
    SessionStats stats;
    stats.active = diagnostic.active;
    stats.sessionNumber = diagnostic.sessionNumber;
    stats.sampleCount = diagnostic.sampleCount;
    stats.targetSamples = diagnostic.targetSamples;
    
    if (diagnostic.active && diagnostic.startTime > 0) {
        unsigned long elapsed = millis() - diagnostic.startTime;
        stats.elapsedTime = elapsed;
        stats.currentFrequency = (elapsed > 0) ? (float)diagnostic.sampleCount * 1000.0 / elapsed : 0.0;
        stats.efficiency = (stats.currentFrequency / 100.0) * 100.0;
        stats.progress = (float)diagnostic.sampleCount * 100.0 / diagnostic.targetSamples;
    } else {
        stats.elapsedTime = 0;
        stats.currentFrequency = 0.0;
        stats.efficiency = 0.0;
        stats.progress = 0.0;
    }
    
    return stats;
}

// Configurer le nombre d'échantillons cible
void setTargetSamples(unsigned long target) {
    diagnostic.targetSamples = target;
    Serial.printf("📊 Cible mise à jour: %lu échantillons\n", target);
}
