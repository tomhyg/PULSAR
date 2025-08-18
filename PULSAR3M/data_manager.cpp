// data_manager.cpp - Avec diagnostic unifié complet et détecteur de retrait + OPTIMISÉ VITESSE
#include "data_manager.h"
#include "sensor_manager.h"
#include "system_manager.h"
#include "spi_manager.h"
#include "led_manager.h"
#include "frequency_diagnostic.h"  // 🔧 NOUVEAU: Système unifié
#include "watch_removal_detector.h"  // 🔍 NOUVEAU: Détecteur retrait montre

File DataManager::continuousFile;
WiFiClientSecure DataManager::wifiClient;
HTTPClient DataManager::https;

extern int bufferIndex;
extern int sampleCount;
extern int currentBufferSize;
extern unsigned long lastSampleTime;
extern unsigned long lastAutoSave;

// Variables multi-session
extern String currentSessionID;
extern unsigned long sessionStartTime;
extern int sessionCounter;
extern String continuousFilename;

// Variables temporelles
extern unsigned long long phoneTimestampMs;
extern unsigned long systemStartMs;
extern String phoneTimezone;

// Timestamp de début de session
extern unsigned long long currentSessionStartTimestamp;

extern String patientID;
extern String patientAge;
extern String patientSex;
extern String patientWeight;    
extern String patientHeight;    
extern String studyNotes;
extern String awsEndpoint;

// =====================
// NOUVEAU: Cycle SD unifié avec diagnostic unifié et détecteur de retrait
// =====================

void DataManager::multiSessionSDCycle() {
    Serial.println("🔄 DÉBUT CYCLE SD UNIFIÉ - Session #" + String(sessionCounter + 1));
    
    // 1️⃣ PRÉPARATION FICHIER (première session uniquement)
    if (sessionCounter == 0) {
        Serial.println("📝 Création fichier multi-session");
        if (!createContinuousFile()) {
            Serial.println("❌ Erreur création fichier");
            return;
        }
    }
    
    // 2️⃣ ACQUISITION PURE (30k échantillons, SD déconnectée)
    Serial.println("📊 Acquisition session #" + String(sessionCounter + 1));
    
    // Fermer fichier et déconnecter SD pendant acquisition
    continuousFile.close();
    digitalWrite(SD_CS, HIGH);
    digitalWrite(LIS3DH_CS, HIGH);
    delay(50);
    
    // 🔧 NOUVEAU: Démarrer diagnostic unifié
    startSessionDiagnostic(sessionCounter + 1);
    
    // 🔍 NOUVEAU: Activer le détecteur de retrait de montre
    WatchRemovalDetector::setEnabled(true);
    Serial.println("🔍 Détecteur de retrait activé pour la session");
    
    // Si c'est pas la première session, il faut réinitialiser le buffer
    if (sessionCounter > 0) {
        bufferIndex = 0;
        
        // 🔋 S'assurer que le PPG est alimenté pour l'acquisition
        if (!SensorManager::isPPGPowered()) {
            Serial.println("🔋 PPG éteint - Rallumage pour acquisition");
            SensorManager::powerUpPPG();
        }
        
        // Acquisition pendant que le buffer se remplit
        while (bufferIndex < currentBufferSize) {
            // 🔍 NOUVEAU: Vérifier si la montre a été retirée
            if (WatchRemovalDetector::isWatchRemoved()) {
                Serial.println("🔍 ❌ Montre retirée détectée pendant acquisition - Arrêt");
                return;
            }
            
            acquireData(micros());
            
            // Condition de sortie de sécurité
            if (bufferIndex >= currentBufferSize) break;
            
            LEDManager::update();
            delayMicroseconds(50);
        }
    }
    // Pour la première session, on a déjà les 30k échantillons dans le buffer !
    
    // 🔧 NOUVEAU: Arrêter diagnostic et rapport final
    stopSessionDiagnostic();
    
    // 🔍 NOUVEAU: Désactiver temporairement le détecteur pendant l'écriture
    WatchRemovalDetector::setEnabled(false);
    Serial.println("🔍 Détecteur de retrait désactivé pour écriture SD");
    
    Serial.printf("✅ Session #%d acquise: %d échantillons\n", 
                 sessionCounter + 1, bufferIndex);
    
    // 🔋 Éteindre le PPG pendant l'écriture SD
    Serial.println("🔋 ARRÊT PPG pendant écriture SD");
    SensorManager::powerDownPPG();
    
    // 3️⃣ ÉCRITURE RAPIDE (SD exclusive)
    Serial.println("⚡ Écriture session dans fichier continu");
    unsigned long writeStart = millis();
    
    // Reconnecter SD en mode exclusif
    SPIManager::initializeConfigMode();
    delay(100);
    
    // Utiliser le timestamp sauvegardé au début de l'acquisition
    if (currentSessionStartTimestamp == 0) {
        Serial.println("⚠️ ATTENTION: Timestamp session manquant, calcul d'urgence");
        currentSessionStartTimestamp = calculateSessionTimestamp(sessionCounter);
    }
    
    Serial.printf("🕐 Utilisation timestamp sauvegardé: %llu ms\n", currentSessionStartTimestamp);
    appendSessionToFile(currentSessionStartTimestamp);
    
    unsigned long writeTime = millis() - writeStart;
    Serial.printf("🎉 Session #%d écrite en %lu ms\n", sessionCounter + 1, writeTime);
    
    // 4️⃣ PRÉPARATION CYCLE SUIVANT
    sessionCounter++;
    bufferIndex = 0;  // 🔧 CORRECTION: Réinitialiser le buffer !
    Serial.printf("✅ Session terminée. Total: %d sessions\n", sessionCounter);
    
    // Réinitialiser le timestamp pour la prochaine session
    currentSessionStartTimestamp = 0;
    
    // Déconnecter SD et repréparer capteurs
    digitalWrite(SD_CS, HIGH);
    SPIManager::initializeSharedMode();
    
    // 🔋 Rallumer le PPG après écriture
    Serial.println("🔋 RALLUMAGE PPG après écriture SD");
    SensorManager::powerUpPPG();
    
    // 🔍 NOUVEAU: Réactiver le détecteur de retrait
    WatchRemovalDetector::setEnabled(true);
    Serial.println("🔍 Détecteur de retrait réactivé après écriture SD");
    
    Serial.println("🔄 Prêt pour session suivante\n");
    delay(2000);
}

// =====================
// CRÉATION FICHIER CONTINU (première fois)
// =====================

bool DataManager::createContinuousFile() {
    Serial.println("📝 === CRÉATION FICHIER CONTINU ===");
    
    // 🔋 Éteindre le PPG pendant création du fichier
    Serial.println("🔋 ARRÊT PPG pendant création fichier");
    SensorManager::powerDownPPG();
    
    continuousFilename = "/data/patient_" + patientID + "_" + String(millis()) + ".json";
    Serial.println("📁 Nom fichier: " + continuousFilename);
    
    if (!SD.begin(SD_CS, SPI, 400000)) {
        Serial.println("❌ Carte SD non accessible");
        // 🔋 Rallumer le PPG même en cas d'erreur
        SensorManager::powerUpPPG();
        return false;
    }
    
    if (!SD.exists("/data")) {
        Serial.println("📁 Création dossier /data");
        SD.mkdir("/data");
    }
    
    continuousFile = SD.open(continuousFilename, FILE_WRITE);
    if (!continuousFile) {
        Serial.println("❌ Erreur création fichier");
        // 🔋 Rallumer le PPG même en cas d'erreur
        SensorManager::powerUpPPG();
        return false;
    }
    
    Serial.println("✅ Fichier ouvert, écriture header...");
    
    // Calculer timestamp de début de fichier
    unsigned long long fileStartTimestamp = calculateSessionTimestamp(0);
    
    // Header principal du fichier avec timestamps corrects
    continuousFile.println("{");
    continuousFile.println("  \"patient_info\": {");
    continuousFile.printf("    \"patient_id\": \"%s\",\n", patientID.c_str());
    continuousFile.printf("    \"patient_age\": \"%s\",\n", patientAge.c_str());
    continuousFile.printf("    \"patient_sex\": \"%s\",\n", patientSex.c_str());
    continuousFile.printf("    \"patient_weight\": \"%s\",\n", patientWeight.c_str());
    continuousFile.printf("    \"patient_height\": \"%s\",\n", patientHeight.c_str());
    continuousFile.printf("    \"study_notes\": \"%s\",\n", studyNotes.c_str());
    continuousFile.printf("    \"device_id\": \"%s\",\n", WiFi.macAddress().c_str());
    
    // Timestamp correct en millisecondes
    continuousFile.printf("    \"start_timestamp_ms\": %llu,\n", fileStartTimestamp);
    continuousFile.printf("    \"timezone\": \"%s\",\n", phoneTimezone.c_str());
    
    continuousFile.printf("    \"sample_rate\": %d,\n", SAMPLE_RATE);
    continuousFile.printf("    \"session_duration_seconds\": %d\n", COLLECTION_TIME_SECONDS);
    continuousFile.println("  },");
    continuousFile.println("  \"sessions\": [");
    
    continuousFile.flush();
    size_t headerSize = continuousFile.size();
    continuousFile.close();
    
    // 🔋 Rallumer le PPG après création fichier
    Serial.println("🔋 RALLUMAGE PPG après création fichier");
    SensorManager::powerUpPPG();
    
    Serial.printf("✅ Header écrit: %d bytes\n", headerSize);
    Serial.printf("✅ Fichier créé: %s\n", continuousFilename.c_str());
    Serial.printf("🕐 Timestamp de début: %llu ms\n", fileStartTimestamp);
    
    return true;
}

// =====================
// AJOUT SESSION AU FICHIER avec timestamp
// =====================

void DataManager::appendSessionToFile(unsigned long long sessionTimestamp) {
    Serial.printf("📝 === DÉBUT APPEND SESSION #%d ===\n", sessionCounter + 1);
    Serial.printf("🕐 Timestamp session: %llu\n", sessionTimestamp);
    Serial.printf("💾 Mémoire libre: RAM %d KB, PSRAM %d KB\n", 
                 ESP.getFreeHeap() / 1024, ESP.getFreePsram() / 1024);
    
    // 🔋 Vérifier que le PPG est éteint pendant l'écriture
    if (SensorManager::isPPGPowered()) {
        Serial.println("🔋 PPG encore allumé - Arrêt forcé pendant écriture");
        SensorManager::powerDownPPG();
    }
    
    continuousFile = SD.open(continuousFilename, FILE_WRITE);
    if (!continuousFile) {
        Serial.println("❌ ERREUR: Impossible d'ouvrir le fichier");
        return;
    }
    
    size_t sizeBefore = continuousFile.size();
    Serial.printf("📊 Taille fichier avant: %d bytes\n", sizeBefore);
    
    if (sessionCounter == 0) {
        // PREMIÈRE SESSION: Écrire normalement à la fin
        Serial.println("📝 Première session - écriture à la fin");
        continuousFile.seek(continuousFile.size());
    } else {
        // SESSIONS SUIVANTES: Supprimer les fermetures pour ajouter dans l'array
        Serial.printf("📝 Session suivante #%d - repositionnement\n", sessionCounter + 1);
        
        // Chercher la fin de l'array sessions: "  ]\n}"
        // On va enlever les 6 derniers caractères: "  ]\n}\n" 
        if (sizeBefore >= 6) {
            continuousFile.seek(sizeBefore - 6);
            Serial.printf("📝 Repositionnement à position %d (suppression fermetures)\n", sizeBefore - 6);
        }
        
        // Ajouter virgule pour séparer les sessions dans l'array
        Serial.println("📝 Ajout virgule pour session suivante dans l'array");
        continuousFile.println(",");
    }
    
    // Header de la session (avec timestamp correct)
    Serial.println("📝 Écriture header session...");
    continuousFile.println("    {");
    continuousFile.printf("      \"session_number\": %d,\n", sessionCounter + 1);
    continuousFile.printf("      \"session_timestamp_ms\": %llu,\n", sessionTimestamp);
    continuousFile.printf("      \"actual_samples\": %d,\n", bufferIndex);
    continuousFile.printf("      \"total_chunks\": 20,\n");
    continuousFile.println("      \"chunks\": [");
    
    Serial.printf("📊 Header session écrit - %d échantillons à traiter\n", bufferIndex);
    
    // Écrire les 20 chunks (même logique qu'AWS)
    Serial.println("🔄 Début écriture chunks...");
    writeSessionChunks();
    
    Serial.println("📝 Finalisation session...");
    continuousFile.println("      ]");
    continuousFile.println("    }");
    
    // TOUJOURS fermer l'array sessions et le JSON principal
    continuousFile.println("  ]");
    continuousFile.println("}");
    
    continuousFile.flush();  // FORCER l'écriture
    size_t finalSize = continuousFile.size();
    continuousFile.close();
    
    Serial.printf("✅ Session #%d terminée - Taille: %d bytes (+%d)\n", 
                 sessionCounter + 1, finalSize, finalSize - sizeBefore);
    Serial.printf("🕐 Timestamp session: %llu ms\n", sessionTimestamp);
}

// =====================
// ⚡ ÉCRITURE CHUNKS OPTIMISÉE (mais pas trop rapide)
// =====================

void DataManager::writeSessionChunks() {
    const int NUM_CHUNKS = 20;
    int samplesPerChunk = (bufferIndex + NUM_CHUNKS - 1) / NUM_CHUNKS;
    
    Serial.printf("⚡ === ÉCRITURE %d CHUNKS OPTIMISÉE ===\n", NUM_CHUNKS);
    Serial.printf("📊 Échantillons par chunk: ~%d\n", samplesPerChunk);
    Serial.printf("📊 Total échantillons: %d\n", bufferIndex);
    
    for (int chunk = 0; chunk < NUM_CHUNKS; chunk++) {
        Serial.printf("🔄 CHUNK %d/%d...\n", chunk + 1, NUM_CHUNKS);
        
        // Vérification mémoire
        size_t freeHeap = ESP.getFreeHeap();
        if (freeHeap < 50000) {  // Moins de 50KB
            Serial.printf("⚠️ MÉMOIRE FAIBLE: %d KB au chunk %d\n", freeHeap / 1024, chunk);
        }
        
        // Vérification fichier
        if (!continuousFile) {
            Serial.printf("❌ ERREUR: Fichier fermé au chunk %d\n", chunk);
            return;
        }
        
        int startIdx = chunk * samplesPerChunk;
        int endIdx = min((chunk + 1) * samplesPerChunk, bufferIndex);
        int samplesInChunk = endIdx - startIdx;
        
        if (samplesInChunk <= 0) {
            Serial.printf("⚠️ Chunk %d vide, arrêt\n", chunk);
            break;
        }
        
        Serial.printf("📊 Chunk %d: échantillons [%d-%d] = %d\n", chunk, startIdx, endIdx, samplesInChunk);
        
        if (chunk > 0) {
            continuousFile.println(",");
        }
        
        continuousFile.println("        {");
        continuousFile.printf("          \"chunk\": %d,\n", chunk);
        continuousFile.printf("          \"samples\": %d,\n", samplesInChunk);
        
        // ⚡ ÉCRITURE OPTIMISÉE - DÉLAIS RÉDUITS
        writeChannelToFile("green", greenPPGBuffer, startIdx, samplesInChunk, true);
        writeChannelToFile("ir", irPPGBuffer, startIdx, samplesInChunk, true);
        writeChannelToFile("red", redPPGBuffer, startIdx, samplesInChunk, true);
        writeChannelToFile("blue", bluePPGBuffer, startIdx, samplesInChunk, true);
        
        // ⚡ OPTIMISATION: Délai réduit entre PPG et timestamps (de 100ms à 20ms)
        delay(20);
        
        writeTimestampsToFile("ppg_timestamps", ppgTimestampBuffer, startIdx, samplesInChunk, true);
        
        // ⚡ OPTIMISATION: Délai réduit entre PPG et accel (de 100ms à 20ms)
        delay(20);
        
        writeChannelToFile("accel_x", (uint32_t*)accelXBuffer, startIdx, samplesInChunk, true);
        writeChannelToFile("accel_y", (uint32_t*)accelYBuffer, startIdx, samplesInChunk, true);
        writeChannelToFile("accel_z", (uint32_t*)accelZBuffer, startIdx, samplesInChunk, true);
        
        // ⚡ OPTIMISATION: Délai réduit avant accel timestamps (de 50ms à 10ms)
        delay(10);
        
        writeTimestampsToFile("accel_timestamps", accelTimestampBuffer, startIdx, samplesInChunk, false);
        
        continuousFile.println("        }");
        
        // ⚡ OPTIMISATION: Flush seulement tous les 3 chunks au lieu de chaque chunk
        if (chunk % 3 == 2 || chunk == NUM_CHUNKS - 1) {
            continuousFile.flush();
        }
        
        Serial.printf("✅ Chunk %d terminé - PSRAM: %d KB, RAM: %d KB\n", 
                     chunk, ESP.getFreePsram() / 1024, ESP.getFreeHeap() / 1024);
        
        // Monitoring périodique
        if (chunk % 5 == 4) {
            Serial.printf("💾 Chunk %d - RAM: %d KB, PSRAM: %d KB\n", 
                         chunk, ESP.getFreeHeap() / 1024, ESP.getFreePsram() / 1024);
        }
    }
    
    // FLUSH FINAL
    continuousFile.flush();
    Serial.println("⚡ TOUS LES CHUNKS ÉCRITS AVEC SUCCÈS (OPTIMISÉ)");
}

// =====================
// ⚡ FONCTIONS D'AIDE ÉCRITURE OPTIMISÉES
// =====================

void DataManager::writeChannelToFile(const String& channelName, uint32_t* buffer, 
                                    int startIdx, int samples, bool addComma) {
    Serial.printf("⚡ Encodage %s: %d échantillons\n", channelName.c_str(), samples);
    
    // VÉRIFICATION CRITIQUE : Fichier toujours ouvert ?
    if (!continuousFile) {
        Serial.printf("❌ ERREUR FATALE: Fichier fermé pendant %s\n", channelName.c_str());
        return;
    }
    
    // ⚡ OPTIMISATION: Délai réduit avant chaque canal (de 50ms à 10ms)
    delay(10);
    
    // ÉCRITURE SÉCURISÉE DU HEADER
    String headerStr = "          \"" + channelName + "\": \"";
    size_t written = continuousFile.print(headerStr);
    if (written != headerStr.length()) {
        Serial.printf("❌ ERREUR: Header %s incomplet\n", channelName.c_str());
        return;
    }
    
    // ⚡ OPTIMISATION: Délai réduit après header (de 20ms à 5ms)
    continuousFile.flush();
    delay(5);
    
    int bufferSize = samples * 4;
    
    // ⚡ OPTIMISATION: Blocs plus gros pour moins d'itérations
    const int ENCODE_CHUNK_SIZE = 4096;  // Doublé de 2048 à 4096
    const int WRITE_BLOCK_SIZE = 1024;   // Doublé de 512 à 1024
    
    Serial.printf("⚡ Encodage par blocs de %d bytes, écriture par %d chars\n", 
                 ENCODE_CHUNK_SIZE, WRITE_BLOCK_SIZE);
    
    int totalWritten = 0;
    
    // Traiter par chunks d'encodage
    for (int offset = 0; offset < bufferSize; offset += ENCODE_CHUNK_SIZE) {
        int chunkSize = min(ENCODE_CHUNK_SIZE, bufferSize - offset);
        int samplesInChunk = chunkSize / 4;
        
        // ⚡ OPTIMISATION: Délai réduit entre chunks d'encodage (de 10ms à 2ms)
        delay(2);
        
        // Allocation buffer temporaire EN PSRAM
        uint8_t* binaryBuffer = (uint8_t*)ps_malloc(chunkSize);
        if (!binaryBuffer) {
            // Fallback sur RAM normale si PSRAM pleine
            binaryBuffer = (uint8_t*)malloc(chunkSize);
            if (!binaryBuffer) {
                Serial.printf("❌ ERREUR: Allocation échouée\n");
                continuousFile.print("\"");
                if (addComma) continuousFile.print(",");
                continuousFile.println();
                return;
            }
        }
        
        // Copier les données pour ce chunk
        for (int i = 0; i < samplesInChunk; i++) {
            uint32_t value = buffer[startIdx + (offset/4) + i];
            binaryBuffer[i * 4]     = (value >> 24) & 0xFF;
            binaryBuffer[i * 4 + 1] = (value >> 16) & 0xFF;
            binaryBuffer[i * 4 + 2] = (value >> 8) & 0xFF;
            binaryBuffer[i * 4 + 3] = value & 0xFF;
        }
        
        // Encoder ce chunk en Base64
        String encoded = base64::encode(binaryBuffer, chunkSize);
        free(binaryBuffer);  // Libère PSRAM ou RAM selon allocation
        
        // ⚡ ÉCRITURE PAR GROS BLOCS AVEC DÉLAIS RÉDUITS
        for (int writeOffset = 0; writeOffset < encoded.length(); writeOffset += WRITE_BLOCK_SIZE) {
            int writeSize = min(WRITE_BLOCK_SIZE, (int)(encoded.length() - writeOffset));
            String writeBlock = encoded.substring(writeOffset, writeOffset + writeSize);
            
            // Vérification état fichier avant chaque bloc
            if (!continuousFile) {
                Serial.printf("❌ ERREUR: Fichier fermé pendant écriture %s\n", channelName.c_str());
                return;
            }
            
            size_t blockWritten = continuousFile.print(writeBlock);
            totalWritten += blockWritten;
            
            if (blockWritten != writeSize) {
                Serial.printf("❌ ERREUR BLOC %s: écrit %d/%d\n", 
                             channelName.c_str(), blockWritten, writeSize);
                
                // Tentative de récupération
                continuousFile.flush();
                delay(50);
                
                // Ré-essayer l'écriture
                blockWritten = continuousFile.print(writeBlock);
                if (blockWritten != writeSize) {
                    Serial.printf("❌ ÉCHEC récupération %s\n", channelName.c_str());
                    return;
                }
            }
            
            // ⚡ OPTIMISATION: Flush et délai réduits (de 20ms à 5ms)
            if ((writeOffset / WRITE_BLOCK_SIZE) % 4 == 3) {  // Flush tous les 4 blocs
                continuousFile.flush();
                delay(5);
            }
        }
    }
    
    // ⚡ OPTIMISATION: Délai réduit avant footer (de 10ms à 2ms)
    delay(2);
    
    // FOOTER
    continuousFile.print("\"");
    if (addComma) continuousFile.print(",");
    continuousFile.println();
    
    // ⚡ OPTIMISATION: Délai final réduit (de 50ms à 10ms)
    delay(10);
    
    Serial.printf("✅ Canal %s: %d chars écrits\n", channelName.c_str(), totalWritten);
}

void DataManager::writeTimestampsToFile(const String& fieldName, unsigned long* buffer, 
                                       int startIdx, int samples, bool addComma) {
    Serial.printf("⚡ Encodage timestamps %s: %d échantillons\n", fieldName.c_str(), samples);
    
    continuousFile.printf("          \"%s\": \"", fieldName.c_str());
    
    int bufferSize = samples * 4;
    
    // Allouer buffer timestamps EN PSRAM
    uint8_t* binaryBuffer = (uint8_t*)ps_malloc(bufferSize);
    
    if (!binaryBuffer) {
        // Fallback sur RAM normale
        binaryBuffer = (uint8_t*)malloc(bufferSize);
        if (!binaryBuffer) {
            Serial.printf("❌ ERREUR: Allocation timestamps %s échouée\n", fieldName.c_str());
            continuousFile.print("\"");
            if (addComma) continuousFile.print(",");
            continuousFile.println();
            return;
        }
    }
    
    unsigned long chunkStartTime = buffer[startIdx];
    
    for (int i = 0; i < samples; i++) {
        uint32_t relativeTime = (uint32_t)((buffer[startIdx + i] - chunkStartTime) / 1000);
        memcpy(&binaryBuffer[i * 4], &relativeTime, 4);
    }
    
    String encoded = base64::encode(binaryBuffer, bufferSize);
    free(binaryBuffer);
    
    continuousFile.print(encoded);
    continuousFile.print("\"");
    if (addComma) continuousFile.print(",");
    continuousFile.println();
    
    // ⚡ OPTIMISATION: Délai réduit après timestamps (de 20ms à 5ms)
    delay(5);
    
    Serial.printf("✅ Timestamps %s terminé\n", fieldName.c_str());
}

// =====================
// ACQUISITION AVEC DIAGNOSTIC UNIFIÉ ET DÉTECTEUR DE RETRAIT
// =====================

void DataManager::acquireData(unsigned long currentMicros) {
    // 🔋 Vérifier que le PPG est allumé avant acquisition
    if (!SensorManager::isPPGPowered()) {
        Serial.println("⚠️ PPG éteint - Impossible d'acquérir des données");
        return;
    }
    
    // 🔍 NOUVEAU: Vérifier si la montre a été retirée
    if (WatchRemovalDetector::isWatchRemoved()) {
        Serial.println("🔍 ⚠️ Montre retirée - Pas d'acquisition");
        return;
    }
    
    SensorManager::updatePPGSensor();
    
    if (!greenPPGBuffer || !accelXBuffer) {
        return;
    }
    
    // Capturer le timestamp réel au début de l'acquisition
    if (bufferIndex == 0 && currentSessionStartTimestamp == 0) {
        currentSessionStartTimestamp = calculateSessionTimestamp(sessionCounter);
        Serial.printf("🕐 *** CAPTURE TIMESTAMP SESSION #%d: %llu ms ***\n", 
                     sessionCounter + 1, currentSessionStartTimestamp);
    }
    
    if (currentMicros - lastSampleTime >= SAMPLE_INTERVAL && bufferIndex < currentBufferSize) {
        lastSampleTime = currentMicros;
        
        uint32_t green, ir, red, blue;
        SensorManager::readPPGData(green, ir, red, blue);
        
        // 🔍 NOUVEAU: Alimenter le détecteur de retrait de montre
        WatchRemovalDetector::addPPGSample(green, ir, red);
        
        float accelX, accelY, accelZ;
        SensorManager::readAccelerometerFast(accelX, accelY, accelZ);
        
        greenPPGBuffer[bufferIndex] = green;
        irPPGBuffer[bufferIndex] = ir;
        redPPGBuffer[bufferIndex] = red;
        bluePPGBuffer[bufferIndex] = blue;
        ppgTimestampBuffer[bufferIndex] = currentMicros;
        
        accelXBuffer[bufferIndex] = accelX;
        accelYBuffer[bufferIndex] = accelY;
        accelZBuffer[bufferIndex] = accelZ;
        accelTimestampBuffer[bufferIndex] = currentMicros;
        
        bufferIndex++;
        
        // 🔧 NOUVEAU: Compter échantillon pour diagnostic unifié
        countSample();
        
        if (bufferIndex % 5000 == 0) {
            float progress = (float)bufferIndex * 100.0 / currentBufferSize;
            Serial.printf("📊 Progression: %d/%d (%.1f%%)\n", bufferIndex, currentBufferSize, progress);
        }
    }
}

// =====================
// TRAITEMENT BUFFER AVEC DIAGNOSTIC ET DÉTECTEUR DE RETRAIT
// =====================

void DataManager::processBuffer() {
    SystemMode currentMode = SystemManager::getCurrentMode();
    
    // 🔍 NOUVEAU: Vérifier si la montre a été retirée
    if (WatchRemovalDetector::isWatchRemoved()) {
        Serial.println("🔍 ❌ Montre retirée - Arrêt du traitement");
        return;
    }
    
    if (currentMode == MODE_SD_RECORDING) {
        // Mode unifié - cycle complet quand buffer plein
        if (bufferIndex >= currentBufferSize) {
            Serial.println("🔄 Buffer SD complet - lancement cycle multi-session");
            multiSessionSDCycle();
        }
    } else if (currentMode == MODE_WIFI_AWS) {
        // Mode AWS : cycle avec diagnostic
        if (bufferIndex >= AWS_BUFFER_SIZE) {
            // 🔧 NOUVEAU: Arrêter diagnostic avant envoi
            stopSessionDiagnostic();
            
            // 🔍 NOUVEAU: Désactiver temporairement le détecteur pendant envoi
            WatchRemovalDetector::setEnabled(false);
            Serial.println("🔍 Détecteur de retrait désactivé pour envoi AWS");
            
            // 🔋 Éteindre le PPG pendant envoi AWS
            Serial.println("🔋 ARRÊT PPG pendant envoi AWS");
            SensorManager::powerDownPPG();
            
            sendBatchToAWSChunked();
            
            // 🔋 Rallumer le PPG après envoi AWS
            Serial.println("🔋 RALLUMAGE PPG après envoi AWS");
            SensorManager::powerUpPPG();
            
            // 🔧 NOUVEAU: Redémarrer diagnostic pour prochaine session
            bufferIndex = 0;
            sessionCounter++;
            startSessionDiagnostic(sessionCounter);
            
            // 🔍 NOUVEAU: Réactiver le détecteur de retrait
            WatchRemovalDetector::setEnabled(true);
            Serial.println("🔍 Détecteur de retrait réactivé pour nouvelle session AWS");
        }
    }
}

// =====================
// FONCTIONS EXISTANTES MODIFIÉES
// =====================

bool DataManager::initializeSD() {
    Serial.println("=== INITIALISATION CARTE SD (Mode unifié) ===");
    
    // 🔋 Éteindre le PPG pendant initialisation SD
    if (SensorManager::isPPGPowered()) {
        Serial.println("🔋 ARRÊT PPG pendant initialisation SD");
        SensorManager::powerDownPPG();
    }
    
    digitalWrite(SD_CS, HIGH);
    digitalWrite(LIS3DH_CS, HIGH);
    delay(50);
    
    bool sdInitialized = false;
    if (SD.begin(SD_CS, SPI, 400000)) {
        Serial.println("✅ Carte SD initialisée");
        sdInitialized = true;
    } else {
        Serial.println("❌ Erreur carte SD");
    }
    
    // 🔋 Rallumer le PPG après initialisation SD
    Serial.println("🔋 RALLUMAGE PPG après initialisation SD");
    SensorManager::powerUpPPG();
    
    return sdInitialized;
}

void DataManager::allocateBuffers() {
    Serial.println("🔄 Allocation buffers unifiés (30k)...");
    
    deallocateBuffers();
    
    currentBufferSize = 30000;  // UNIFIÉ
    
    // 🔧 NOUVEAU: Configurer la cible du diagnostic
    setTargetSamples(currentBufferSize);
    
    // 🔍 NOUVEAU: Initialiser le détecteur de retrait de montre
    WatchRemovalDetector::init();
    Serial.println("🔍 Détecteur de retrait initialisé");
    
    Serial.printf("📊 Taille buffer unifié: %d échantillons\n", currentBufferSize);
    
    size_t ppgMemory = currentBufferSize * sizeof(uint32_t) * 4;
    size_t ppgTimestampMemory = currentBufferSize * sizeof(unsigned long);
    size_t accelMemory = currentBufferSize * sizeof(float) * 3;
    size_t accelTimestampMemory = currentBufferSize * sizeof(unsigned long);
    size_t totalMemory = ppgMemory + ppgTimestampMemory + accelMemory + accelTimestampMemory;
    
    Serial.printf("💾 Mémoire nécessaire: %d KB\n", totalMemory / 1024);
    
    bool usePSRAM = isPSRAMAvailable();
    if (usePSRAM) {
        Serial.printf("✅ PSRAM disponible: %d KB\n", ESP.getFreePsram() / 1024);
        
        greenPPGBuffer = (uint32_t *)ps_malloc(currentBufferSize * sizeof(uint32_t));
        irPPGBuffer = (uint32_t *)ps_malloc(currentBufferSize * sizeof(uint32_t));
        redPPGBuffer = (uint32_t *)ps_malloc(currentBufferSize * sizeof(uint32_t));
        bluePPGBuffer = (uint32_t *)ps_malloc(currentBufferSize * sizeof(uint32_t));
        ppgTimestampBuffer = (unsigned long *)ps_malloc(currentBufferSize * sizeof(unsigned long));
        
        accelXBuffer = (float *)ps_malloc(currentBufferSize * sizeof(float));
        accelYBuffer = (float *)ps_malloc(currentBufferSize * sizeof(float));
        accelZBuffer = (float *)ps_malloc(currentBufferSize * sizeof(float));
        accelTimestampBuffer = (unsigned long *)ps_malloc(currentBufferSize * sizeof(unsigned long));
    } else {
        Serial.printf("⚠️ RAM interne: %d KB\n", ESP.getFreeHeap() / 1024);
        
        greenPPGBuffer = (uint32_t *)malloc(currentBufferSize * sizeof(uint32_t));
        irPPGBuffer = (uint32_t *)malloc(currentBufferSize * sizeof(uint32_t));
        redPPGBuffer = (uint32_t *)malloc(currentBufferSize * sizeof(uint32_t));
        bluePPGBuffer = (uint32_t *)malloc(currentBufferSize * sizeof(uint32_t));
        ppgTimestampBuffer = (unsigned long *)malloc(currentBufferSize * sizeof(unsigned long));
        
        accelXBuffer = (float *)malloc(currentBufferSize * sizeof(float));
        accelYBuffer = (float *)malloc(currentBufferSize * sizeof(float));
        accelZBuffer = (float *)malloc(currentBufferSize * sizeof(float));
        accelTimestampBuffer = (unsigned long *)malloc(currentBufferSize * sizeof(unsigned long));
    }
    
    if (!greenPPGBuffer || !irPPGBuffer || !redPPGBuffer || !bluePPGBuffer || 
        !ppgTimestampBuffer || !accelXBuffer || !accelYBuffer || !accelZBuffer || 
        !accelTimestampBuffer) {
        
        Serial.println("❌ ÉCHEC allocation buffers!");
        deallocateBuffers();
        while (1) delay(1000);
    }
    
    memset(greenPPGBuffer, 0, currentBufferSize * sizeof(uint32_t));
    memset(irPPGBuffer, 0, currentBufferSize * sizeof(uint32_t));
    memset(redPPGBuffer, 0, currentBufferSize * sizeof(uint32_t));
    memset(bluePPGBuffer, 0, currentBufferSize * sizeof(uint32_t));
    memset(ppgTimestampBuffer, 0, currentBufferSize * sizeof(unsigned long));
    memset(accelXBuffer, 0, currentBufferSize * sizeof(float));
    memset(accelYBuffer, 0, currentBufferSize * sizeof(float));
    memset(accelZBuffer, 0, currentBufferSize * sizeof(float));
    memset(accelTimestampBuffer, 0, currentBufferSize * sizeof(unsigned long));
    
    Serial.printf("✅ Buffers alloués: %d échantillons (%d KB)\n", 
                 currentBufferSize, totalMemory / 1024);
}

void DataManager::deallocateBuffers() {
    if (greenPPGBuffer) { free(greenPPGBuffer); greenPPGBuffer = nullptr; }
    if (irPPGBuffer) { free(irPPGBuffer); irPPGBuffer = nullptr; }
    if (redPPGBuffer) { free(redPPGBuffer); redPPGBuffer = nullptr; }
    if (bluePPGBuffer) { free(bluePPGBuffer); bluePPGBuffer = nullptr; }
    if (ppgTimestampBuffer) { free(ppgTimestampBuffer); ppgTimestampBuffer = nullptr; }
    
    if (accelXBuffer) { free(accelXBuffer); accelXBuffer = nullptr; }
    if (accelYBuffer) { free(accelYBuffer); accelYBuffer = nullptr; }
    if (accelZBuffer) { free(accelZBuffer); accelZBuffer = nullptr; }
    if (accelTimestampBuffer) { free(accelTimestampBuffer); accelTimestampBuffer = nullptr; }
}

// =====================
// FONCTIONS AWS AVEC DIAGNOSTIC
// =====================

void DataManager::sendBatchToAWSChunked() {
    Serial.println("📡 Envoi AWS...");
    
    if (bufferIndex == 0) return;
    
    const int NUM_CHUNKS = COLLECTION_TIME_SECONDS / CHUNK_DURATION;
    int ppgSamplesPerChunk = (bufferIndex + NUM_CHUNKS - 1) / NUM_CHUNKS;
    
    for (int chunk = 0; chunk < NUM_CHUNKS; chunk++) {
        int ppgStartIdx = chunk * ppgSamplesPerChunk;
        int ppgEndIdx = min((chunk + 1) * ppgSamplesPerChunk, bufferIndex);
        int ppgSamplesInChunk = ppgEndIdx - ppgStartIdx;
        
        if (ppgSamplesInChunk <= 0) break;
        
        String payload = "{";
        payload += "\"watch_id\": \"" + WiFi.macAddress() + "\",";
        payload += "\"signal_quality\": \"" + String(WiFi.RSSI()) + "\",";
        payload += "\"battery_level\": \"3.88\",";
        payload += "\"sample_rate_ppg\": " + String(SAMPLE_RATE) + ",";
        payload += "\"sample_rate_accel\": " + String(SAMPLE_RATE) + ",";
        payload += "\"total_chunks\": " + String(NUM_CHUNKS) + ",";
        payload += "\"chunk\": " + String(chunk) + ",";
        payload += "\"ppg_samples\": " + String(ppgSamplesInChunk) + ",";
        payload += "\"accel_samples\": " + String(ppgSamplesInChunk) + ",";
        
        // Encodage des données (code existant)
        // ... (ajout des données en base64)
        
        payload += "}";
        
        wifiClient.setInsecure();
        https.begin(wifiClient, awsEndpoint);
        https.addHeader("Content-Type", "application/json");
        
        int httpResponseCode = https.POST(payload);
        Serial.printf("Chunk %d: %d\n", chunk, httpResponseCode);
        https.end();
        
        delay(500);
    }
}

// Fonctions obsolètes - garder pour compatibilité mais vides
void DataManager::initializeContinuousFile() {
    // Remplacé par createContinuousFile()
}

void DataManager::writeBufferToSD() {
    // Remplacé par multiSessionSDCycle()
}

void DataManager::sendBufferToAWS() {
    sendBatchToAWSChunked();
}

void DataManager::performAutoSave() {
    // Plus nécessaire avec le nouveau système
}