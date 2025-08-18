// frequency_diagnostic.h - Système de diagnostic de fréquence unifié
#ifndef FREQUENCY_DIAGNOSTIC_H
#define FREQUENCY_DIAGNOSTIC_H

#include <Arduino.h>

// Structure pour les statistiques de session
struct SessionStats {
    bool active;                    // Diagnostic actif
    int sessionNumber;              // Numéro de session
    unsigned long sampleCount;      // Nombre d'échantillons acquis
    unsigned long targetSamples;    // Nombre d'échantillons cible
    unsigned long elapsedTime;      // Temps écoulé (ms)
    float currentFrequency;         // Fréquence actuelle (Hz)
    float efficiency;               // Efficacité (%)
    float progress;                 // Progression (%)
};

// Fonctions principales du diagnostic
void startSessionDiagnostic(int sessionNum);    // Démarrer diagnostic pour session
void countSample();                             // Compter un échantillon
void reportSessionProgress();                   // Rapport périodique
void stopSessionDiagnostic();                   // Arrêter et rapport final
bool isSessionDiagnosticActive();               // Vérifier si actif
SessionStats getSessionStats();                 // Obtenir statistiques actuelles
void setTargetSamples(unsigned long target);    // Configurer cible

#endif