// test_detector.cpp - Fichier de test pour le détecteur de retrait (SANS VARIANCE)
// À inclure uniquement pour les tests de développement

#include "config.h"  // IMPORTANT: Inclure config.h en premier
#include "watch_removal_detector.h"

// Fonction de test pour simuler différents scénarios
void testWatchRemovalDetector() {
  Serial.println("🔍 === TEST DÉTECTEUR DE RETRAIT (SANS VARIANCE) ===");

  // Initialiser le détecteur
  WatchRemovalDetector::init();
  WatchRemovalDetector::setEnabled(true);

  // Test 1: Signal normal
  Serial.println("🔍 Test 1: Simulation signal normal");
  for (int i = 0; i < 600; i++) {                  // 6 secondes de données
    uint32_t green = 30000 + random(-5000, 5000);  // Signal normal avec variation
    uint32_t ir = 28000 + random(-4000, 4000);
    uint32_t red = 25000 + random(-3000, 3000);

    WatchRemovalDetector::addPPGSample(green, ir, red);

    if (i % 100 == 0) {
      WatchRemovalDetector::update();
    }

    delay(10);  // Simuler 100Hz
  }

  WatchRemovalDetector::printDiagnostics();
  Serial.println("🔍 État après signal normal: " + String(WatchRemovalDetector::getState()));

  // Test 2: Signal trop faible (montre retirée)
  Serial.println("\n🔍 Test 2: Simulation signal trop faible");
  for (int i = 0; i < 600; i++) {              // 6 secondes de données
    uint32_t green = 500 + random(-200, 200);  // Signal très faible
    uint32_t ir = 400 + random(-100, 100);
    uint32_t red = 300 + random(-50, 50);

    WatchRemovalDetector::addPPGSample(green, ir, red);

    if (i % 100 == 0) {
      WatchRemovalDetector::update();
      if (WatchRemovalDetector::isWatchRemoved()) {
        Serial.println("🔍 ✅ Détection correcte: Montre retirée");
        break;
      }
    }

    delay(10);
  }

  WatchRemovalDetector::printDiagnostics();

  // Test 3: Signal saturé
  Serial.println("\n🔍 Test 3: Simulation signal saturé");
  WatchRemovalDetector::reset();
  WatchRemovalDetector::setEnabled(true);

  for (int i = 0; i < 600; i++) {                  // 6 secondes de données
    uint32_t green = 90000 + random(-1000, 1000);  // Signal saturé
    uint32_t ir = 91000 + random(-500, 500);
    uint32_t red = 89000 + random(-800, 800);

    WatchRemovalDetector::addPPGSample(green, ir, red);

    if (i % 100 == 0) {
      WatchRemovalDetector::update();
      if (WatchRemovalDetector::isWatchRemoved()) {
        Serial.println("🔍 ✅ Détection correcte: Signal saturé");
        break;
      }
    }

    delay(10);
  }

  WatchRemovalDetector::printDiagnostics();

  // Test 4: Calibration automatique
  Serial.println("\n🔍 Test 4: Calibration automatique");
  WatchRemovalDetector::reset();
  WatchRemovalDetector::setEnabled(true);

  // Générer un signal "normal" pour calibration
  for (int i = 0; i < 500; i++) {
    uint32_t green = 35000 + random(-8000, 8000);
    uint32_t ir = 32000 + random(-7000, 7000);
    uint32_t red = 30000 + random(-6000, 6000);

    WatchRemovalDetector::addPPGSample(green, ir, red);
    delay(10);
  }

  // Effectuer la calibration
  WatchRemovalDetector::calibrateThresholds();

  Serial.println("🔍 === FIN TESTS DÉTECTEUR ===");
}

// Fonction pour tester différents seuils
void testCustomThresholds() {
  Serial.println("🔍 === TEST SEUILS PERSONNALISÉS ===");

  // Seuils très sensibles
  WatchRemovalDetector::setCustomThresholds(2000, 50000, 10000, 60000);
  Serial.println("🔍 Seuils très sensibles définis");

  // Seuils très tolérants
  WatchRemovalDetector::setCustomThresholds(100, 200000, 1000, 95000);
  Serial.println("🔍 Seuils très tolérants définis");

  // Retour aux seuils par défaut
  WatchRemovalDetector::setCustomThresholds(
    AMPLITUDE_THRESHOLD_LOW,
    AMPLITUDE_THRESHOLD_HIGH,
    MEAN_THRESHOLD_LOW,
    MEAN_THRESHOLD_HIGH);
  Serial.println("🔍 Seuils par défaut restaurés");
}

// Fonction pour simuler une session complète
void simulateFullSession() {
  Serial.println("🔍 === SIMULATION SESSION COMPLÈTE ===");

  WatchRemovalDetector::init();
  WatchRemovalDetector::setEnabled(true);

  // Phase 1: 30 secondes de signal normal
  Serial.println("🔍 Phase 1: Signal normal (30 sec)");
  for (int i = 0; i < 3000; i++) {
    uint32_t green = 30000 + random(-6000, 6000);
    uint32_t ir = 28000 + random(-5000, 5000);
    uint32_t red = 25000 + random(-4000, 4000);

    WatchRemovalDetector::addPPGSample(green, ir, red);

    if (i % 100 == 0) {
      WatchRemovalDetector::update();
    }

    delay(10);
  }

  // Phase 2: 5 secondes de signal dégradé
  Serial.println("🔍 Phase 2: Signal dégradé (5 sec)");
  for (int i = 0; i < 500; i++) {
    uint32_t green = 15000 + random(-3000, 3000);
    uint32_t ir = 14000 + random(-2000, 2000);
    uint32_t red = 13000 + random(-1000, 1000);

    WatchRemovalDetector::addPPGSample(green, ir, red);

    if (i % 100 == 0) {
      WatchRemovalDetector::update();
    }

    delay(10);
  }

  // Phase 3: Signal de retrait
  Serial.println("🔍 Phase 3: Signal de retrait");
  for (int i = 0; i < 500; i++) {
    uint32_t green = 200 + random(-100, 100);
    uint32_t ir = 150 + random(-50, 50);
    uint32_t red = 100 + random(-30, 30);

    WatchRemovalDetector::addPPGSample(green, ir, red);

    if (i % 100 == 0) {
      WatchRemovalDetector::update();
      if (WatchRemovalDetector::isWatchRemoved()) {
        Serial.println("🔍 ✅ Retrait détecté à l'échantillon " + String(i));
        break;
      }
    }

    delay(10);
  }

  WatchRemovalDetector::printDiagnostics();

  Serial.println("🔍 === FIN SIMULATION ===");
}

// Fonction utilitaire pour afficher les statistiques en temps réel
void printRealtimeStats() {
  if (WatchRemovalDetector::isEnabled()) {
    SignalStats stats = WatchRemovalDetector::getLastStats();
    DetectorState state = WatchRemovalDetector::getState();

    Serial.printf("🔍 STATS: État=%s, Amp=%u, Moy=%u\n",
                  (state == DETECTOR_NORMAL) ? "NORMAL" : (state == DETECTOR_SUSPICIOUS) ? "SUSPECT"
                                                        : (state == DETECTOR_REMOVED)    ? "RETIRÉE"
                                                                                         : "RESTART",
                  stats.amplitude, stats.mean);
  }
}

// À appeler dans le setup() pour lancer les tests
void runDetectorTests() {
  Serial.println("🔍 === DÉMARRAGE TESTS DÉTECTEUR (SANS VARIANCE) ===");

  delay(2000);  // Attendre que le système soit prêt

  testWatchRemovalDetector();
  delay(1000);

  testCustomThresholds();
  delay(1000);

  simulateFullSession();

  Serial.println("🔍 === TOUS LES TESTS TERMINÉS ===");
}

// Fonction à appeler périodiquement dans loop() pour monitoring
void monitorDetector() {
  static unsigned long lastMonitor = 0;
  unsigned long currentTime = millis();

  if (currentTime - lastMonitor > 5000) {  // Toutes les 5 secondes
    lastMonitor = currentTime;

    if (WatchRemovalDetector::isEnabled()) {
      printRealtimeStats();
    }
  }
}