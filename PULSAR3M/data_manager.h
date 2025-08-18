// data_manager.h - Header mis à jour avec système unifié et fonctions d'encodage
#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include "config.h"

class DataManager {
public:
    static bool initializeSD();
    static void acquireData(unsigned long currentMicros);
    static void allocateBuffers();
    static void deallocateBuffers();
    
    // 🔧 NOUVELLES FONCTIONS STATE MACHINE NON-BLOQUANTES
    static void initializeFirstSession();          // Initialiser première session
    static void prepareDedicatedAcquisition();     // Préparer acquisition dédiée
    static bool isSessionComplete();               // Vérifier si session terminée
    static bool processSessionData();              // Traiter données session
    static void prepareNextSession();              // Préparer session suivante
    static void performSDWriting();                // Écriture SD optimisée
    static void performAWSSending();               // Envoi AWS optimisé
    
    // 🔧 ANCIENNES FONCTIONS UNIFIÉES (obsolètes)
    static void startUnifiedAcquisitionCycle();    // → Obsolète
    static void startUnifiedSDCycle();             // → Obsolète  
    static void startUnifiedAWSCycle();            // → Obsolète
    static void performDedicatedAcquisition(int sessionNum);  // → Obsolète
    
    // FONCTIONS SD EXISTANTES
    static bool createContinuousFile();
    static void appendSessionToFile(unsigned long long sessionTimestamp);
    static void writeSessionChunks();
    static void writeChannelToFile(const String& channelName, uint32_t* buffer, 
                                  int startIdx, int samples, bool addComma);
    static void writeTimestampsToFile(const String& fieldName, unsigned long* buffer, 
                                     int startIdx, int samples, bool addComma);
    
    // FONCTIONS AWS EXISTANTES
    static void sendBatchToAWSChunked();
    
    // 🔧 NOUVELLES FONCTIONS D'ENCODAGE (pour AWS)
    static String encodeChannelData(uint32_t* buffer, int startIdx, int samples);
    static String encodeFloatChannelData(float* buffer, int startIdx, int samples);
    static String encodeTimestampData(unsigned long* buffer, int startIdx, int samples);
    
    // 🔧 FONCTIONS OBSOLÈTES (gardées pour compatibilité)
    static void multiSessionSDCycle();           // → Redirige vers startUnifiedAcquisitionCycle()
    static void processBuffer();                 // → Obsolète avec système unifié
    static void initializeContinuousFile();      // → Vide
    static void writeBufferToSD();               // → Vide
    static void sendBufferToAWS();               // → Redirige vers sendBatchToAWSChunked()
    static void performAutoSave();               // → Vide
    
private:
    static File continuousFile;
    static WiFiClientSecure wifiClient;
    static HTTPClient https;
};

#endif