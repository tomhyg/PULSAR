// config.h - Configuration optimisée pour PSRAM 8MB avec gestion temporelle et détecteur de retrait + BATTERIE
#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>
#include <Base64.h>
#include <SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library.h>  // 🔋 NOUVEAU: Fuel Gauge
#include "dw_MAX86916.h"
#include "secrets_aws_rest.h"

// =====================
// Configuration pins
// =====================
#define SD_CLK 48
#define SD_MISO 21
#define SD_MOSI 47
#define SD_CS 45

#define LIS3DH_CLK 48
#define LIS3DH_MISO 21
#define LIS3DH_MOSI 47
#define LIS3DH_CS 14

#define LED_PIN 5
#define NUM_LEDS 1
#define BRIGHTNESS 50

// =====================
// Configuration système OPTIMISÉE PSRAM - UNIFIÉE
// =====================
#define SAMPLE_RATE 100              // Fréquence d'échantillonnage
#define SAMPLE_INTERVAL 10000        // 10ms = 100Hz
#define COLLECTION_TIME_SECONDS 300  // 5 minutes

// Buffer unifié 30k pour SD et AWS
#define SD_BUFFER_SIZE 30000   // 30k échantillons pour SD
#define AWS_BUFFER_SIZE 30000  // 30k échantillons pour AWS

// Intervalles de temps
#define LED_BLINK_INTERVAL 1000
#define AUTO_SAVE_INTERVAL 30000  // 30 secondes

// 🔋 NOUVEAU: Configuration batterie
#define BATTERY_CHECK_INTERVAL 10000     // Vérification toutes les 10 secondes
#define BATTERY_LOW_THRESHOLD 20.0       // Seuil batterie faible (20%)
#define BATTERY_CRITICAL_THRESHOLD 10.0  // Seuil batterie critique (10%)

// =====================
// Configuration AWS (cycles)
// =====================
#define CHUNK_DURATION 15          // 15 secondes par chunk
#define MAX_CHUNKS_PER_SESSION 20  // Maximum de chunks par session

// =====================
// Configuration WiFi optimisée
// =====================
#define WIFI_CONNECT_TIMEOUT_MS 10000  // 10 secondes max par réseau
#define WIFI_RETRY_DELAY_MS 2000       // 2 secondes entre tentatives
#define WIFI_MAX_NETWORKS 5            // Limite le nombre de réseaux testés

// =====================
// 🔍 Configuration détecteur de retrait (SANS VARIANCE)
// =====================
#define DETECTOR_WINDOW_SIZE 500        // 5 secondes à 100Hz
#define DETECTOR_CHECK_INTERVAL 2000    // Vérification toutes les 1 seconde
#define DETECTOR_RESTART_DELAY 10000     // Délai avant redémarrage (5 secondes)

// Seuils de détection (SANS VARIANCE)
#define AMPLITUDE_THRESHOLD_LOW 500    // Seuil minimum d'amplitude PPG
#define AMPLITUDE_THRESHOLD_HIGH 400000 // Seuil maximum d'amplitude PPG
#define MEAN_THRESHOLD_LOW 2000         // Seuil minimum de moyenne PPG
#define MEAN_THRESHOLD_HIGH 400000       // Seuil maximum de moyenne PPG

// =====================
// Énumérations
// =====================
enum SystemMode {
  MODE_CONFIG,        // Point d'accès pour configuration
  MODE_SD_RECORDING,  // Enregistrement continu sur SD - UNIFIÉ
  MODE_WIFI_AWS       // Envoi par WiFi vers AWS
};

enum LEDState {
  LED_CONFIG_AP,     // Orange clignotant - Point d'accès actif
  LED_WIFI_CONNECT,  // Vert clignotant - Connexion WiFi
  LED_SD_RECORD,     // Bleu clignotant - Enregistrement SD
  LED_AWS_SEND,      // Rouge clignotant - Envoi AWS
  LED_BATTERY_LOW,   // Rouge clignotant rapide - Batterie faible
  LED_ERROR,         // Rouge fixe - Erreur
  LED_OFF
};

enum SPIMode {
  SPI_CONFIG_SD_ONLY,  // Configuration : SD seule sur le bus
  SPI_SHARED_MODE      // Acquisition : Bus partagé SD + LIS3DH
};

// 🔍 États du détecteur de retrait
enum DetectorState {
    DETECTOR_NORMAL,        // Signal normal, montre portée
    DETECTOR_SUSPICIOUS,    // Signal suspect, surveillance renforcée
    DETECTOR_REMOVED,       // Montre retirée détectée
    DETECTOR_RESTARTING     // En cours de redémarrage
};

// 🔍 Structure pour les statistiques de signal (SANS VARIANCE)
struct SignalStats {
    uint32_t minValue;
    uint32_t maxValue;
    uint32_t amplitude;
    uint32_t mean;
    bool isValid;
};

// 🔋 NOUVEAU: Structure pour les données de batterie
struct BatteryInfo {
    float voltage;          // Tension en volts
    float percentage;       // Pourcentage de charge (0-100%)
    float changeRate;       // Taux de décharge (%/h)
    bool isCharging;        // En cours de charge
    bool isLowBattery;      // Batterie faible
    bool isCriticalBattery; // Batterie critique
    String status;          // Statut texte
    bool isValid;           // Données valides
};

// 🔋 NOUVEAU: États de la batterie
enum BatteryState {
    BATTERY_NORMAL,       // Batterie normale (>30%)
    BATTERY_LOW,          // Batterie faible (15-30%)
    BATTERY_CRITICAL,     // Batterie critique (<15%)
    BATTERY_CHARGING,     // En cours de charge
    BATTERY_ERROR         // Erreur capteur
};

// =====================
// Variables globales - Configuration
// =====================
extern String patientID;
extern String patientAge;
extern String patientSex;
extern String patientWeight;
extern String patientHeight;
extern String studyNotes;
extern String sessionDatetime;
extern String wifiSSID;
extern String wifiPassword;
extern String awsEndpoint;
extern SystemMode selectedMode;

// Variables temporelles pour synchronisation téléphone
extern unsigned long long phoneTimestampMs;  // 64 bits pour les gros timestamps
extern unsigned long systemStartMs;          // Moment où le système a démarré (millis())
extern String phoneTimezone;                 // Timezone du téléphone

// Variable pour capturer le timestamp de début de session
extern unsigned long long currentSessionStartTimestamp;

// Variables de session - pour multi-session
extern String currentSessionID;
extern unsigned long sessionStartTime;
extern int sessionCounter;         // Compteur de sessions
extern String continuousFilename;  // Nom du fichier continu

// Variables d'acquisition - Pointeurs pour allocation dynamique
extern uint32_t *greenPPGBuffer;
extern uint32_t *irPPGBuffer;
extern uint32_t *redPPGBuffer;
extern uint32_t *bluePPGBuffer;
extern unsigned long *ppgTimestampBuffer;

extern float *accelXBuffer;
extern float *accelYBuffer;
extern float *accelZBuffer;
extern unsigned long *accelTimestampBuffer;

extern int bufferIndex;
extern int sampleCount;
extern int currentBufferSize;  // Taille effective du buffer selon le mode

// Timing
extern unsigned long lastSampleTime;
extern unsigned long lastAutoSave;

// =====================
// Fonctions utilitaires temporelles
// =====================
// Calcule le timestamp réel d'une session basé sur l'heure du téléphone
inline unsigned long long calculateSessionTimestamp(int sessionNumber) {
  if (phoneTimestampMs == 0 || systemStartMs == 0) {
    Serial.println("⚠️ ATTENTION: Pas de sync temporelle, utilisation millis()");
    return millis();  // Fallback sur millis si pas de sync
  }

  // Temps écoulé depuis le démarrage système
  unsigned long elapsedMs = millis() - systemStartMs;

  // Vérifier si phoneTimestampMs est en secondes ou millisecondes
  unsigned long long phoneTimestampMsCorrect = phoneTimestampMs;
  if (phoneTimestampMs < 1577836800000ULL) {            // Si moins de 2020 en millisecondes
    phoneTimestampMsCorrect = phoneTimestampMs * 1000;  // Convertir en millisecondes
    Serial.printf("🔧 Correction: timestamp converti de %llu à %llu ms\n",
                  phoneTimestampMs, phoneTimestampMsCorrect);
  }

  // Calculer le timestamp réel EN MILLISECONDES (64 bits)
  unsigned long long realTimestampMs = phoneTimestampMsCorrect + elapsedMs;

  // Debug
  Serial.printf("🕐 Calcul timestamp session %d:\n", sessionNumber);
  Serial.printf("   📱 Timestamp téléphone: %llu ms\n", phoneTimestampMs);
  Serial.printf("   🔧 Timestamp corrigé: %llu ms\n", phoneTimestampMsCorrect);
  Serial.printf("   ⏱️ Système démarré: %lu ms\n", systemStartMs);
  Serial.printf("   ⏳ Temps écoulé: %lu ms (%.1f min)\n", elapsedMs, elapsedMs / 60000.0);
  Serial.printf("   🎯 Timestamp calculé: %llu ms\n", realTimestampMs);

  // Vérification cohérence (timestamp > 2020)
  if (realTimestampMs < 1577836800000ULL) {
    Serial.printf("⚠️ ATTENTION: Timestamp semble incorrect: %llu ms\n", realTimestampMs);
    Serial.printf("⚠️ Cela correspond à: %llu secondes\n", realTimestampMs / 1000);
  } else {
    Serial.printf("✅ Timestamp valide: %llu ms\n", realTimestampMs);
  }

  return realTimestampMs;  // Retourner en millisecondes
}

// Fonctions de formatage timestamp simplifiées
inline String formatTimeFromTimestamp(unsigned long long timestampMs) {
  return String((unsigned long)(timestampMs & 0xFFFFFFFF));
}

inline String formatDateFromTimestamp(unsigned long long timestampMs) {
  return String((unsigned long)(timestampMs & 0xFFFFFFFF));
}

inline String formatDateTimeFromTimestamp(unsigned long long timestampMs) {
  return String((unsigned long)(timestampMs & 0xFFFFFFFF));
}

// =====================
// Fonctions utilitaires mémoire
// =====================
inline size_t getTargetBufferSize(SystemMode mode) {
  return 30000;  // UNIFIÉ: 30k pour les deux modes
}

inline bool isPSRAMAvailable() {
  return psramFound() && (ESP.getFreePsram() > 1024 * 1024);  // Au moins 1MB libre
}

#endif