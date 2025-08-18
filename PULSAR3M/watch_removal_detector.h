// watch_removal_detector.h - Système de détection de retrait de montre (SANS VARIANCE)
#ifndef WATCH_REMOVAL_DETECTOR_H
#define WATCH_REMOVAL_DETECTOR_H

#include <Arduino.h>
#include "config.h"  // IMPORTANT: Inclure config.h pour les définitions communes

// Les types DetectorState et SignalStats sont définis dans config.h
// Les constantes DETECTOR_* sont définies dans config.h

class WatchRemovalDetector {
public:
    static void init();
    static void update();
    static void addPPGSample(uint32_t green, uint32_t ir, uint32_t red);
    static bool isWatchRemoved();
    static DetectorState getState();
    static void reset();
    static void setEnabled(bool enabled);
    static bool isEnabled();
    static SignalStats getLastStats();
    static void printDiagnostics();
    
    // Fonctions de calibration des seuils (SANS VARIANCE)
    static void calibrateThresholds();
    static void setCustomThresholds(uint32_t ampLow, uint32_t ampHigh, 
                                   uint32_t meanLow, uint32_t meanHigh);
    
private:
    static uint32_t ppgBuffer[DETECTOR_WINDOW_SIZE];
    static int bufferIndex;
    static int sampleCount;
    static unsigned long lastCheckTime;
    static unsigned long removalDetectedTime;
    static DetectorState currentState;
    static bool enabled;
    static SignalStats lastStats;
    static int consecutiveAnomalies;
    static bool bufferFull;
    
    // Seuils ajustables (SANS VARIANCE)
    static uint32_t amplitudeThresholdLow;
    static uint32_t amplitudeThresholdHigh;
    static uint32_t meanThresholdLow;
    static uint32_t meanThresholdHigh;
    
    static SignalStats calculateStats();
    static bool isSignalAnomalous(const SignalStats& stats);
    static void handleRemovalDetected();
    static void performRestart();
};

#endif