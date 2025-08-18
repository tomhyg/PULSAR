// sensor_manager.cpp - Avec gestion d'alimentation PPG
#include "sensor_manager.h"
#include "spi_manager.h"

MAX86916 SensorManager::particleSensor;
bool SensorManager::ppgPowered = false;  // NOUVEAU: État d'alimentation PPG

bool SensorManager::initPPGSensor() {
    if (!particleSensor.begin()) {
        return false;
    }
    
    Serial.println("MAX86916 initialise");
    
    particleSensor.setup(0, 0xFF, 1, 3, SAMPLE_RATE, 220, 32768);
    particleSensor.enableALCOVF();
    particleSensor.enableFIFORollover();
    particleSensor.enableCrosstalkCancellation();
    
    // Nettoyer le FIFO
    int discardedSamples = 0;
    while (particleSensor.available()) {
        particleSensor.getFIFOGreen();
        particleSensor.getFIFOIR();
        particleSensor.getFIFORed();
        particleSensor.getFIFOBlue();
        particleSensor.nextSample();
        discardedSamples++;
    }
    
    if (discardedSamples > 0) {
        Serial.printf("FIFO PPG nettoyé, %d échantillons ignorés au démarrage\n", discardedSamples);
    }
    
    // NOUVEAU: Marquer comme alimenté
    ppgPowered = true;
    Serial.println("✅ PPG alimenté et prêt");
    
    return true;
}

// 🔋 NOUVEAU: Éteindre le PPG pour économiser l'énergie
void SensorManager::powerDownPPG() {
    if (!ppgPowered) {
        Serial.println("🔋 PPG déjà éteint");
        return;
    }
    
    Serial.println("🔋 ARRÊT PPG - Économie d'énergie");
    
    // Méthode 1: Shutdown mode (recommandé)
    particleSensor.shutDown();
    
    // Méthode 2: Réduire tous les courants LED à 0
    particleSensor.setPulseAmplitudeRed(0);
    particleSensor.setPulseAmplitudeIR(0);
    particleSensor.setPulseAmplitudeGreen(0);
    particleSensor.setPulseAmplitudeBlue(0);
    
    ppgPowered = false;
    Serial.println("✅ PPG éteint - Économie d'énergie active");
}

// 🔋 NOUVEAU: Rallumer le PPG après économie d'énergie
void SensorManager::powerUpPPG() {
    if (ppgPowered) {
        Serial.println("🔋 PPG déjà allumé");
        return;
    }
    
    Serial.println("🔋 DÉMARRAGE PPG - Sortie mode économie");
    
    // Méthode 1: Sortir du shutdown mode
    particleSensor.wakeUp();
    
    // Méthode 2: Restaurer les courants LED
    particleSensor.setPulseAmplitudeRed(220);
    particleSensor.setPulseAmplitudeIR(220);
    particleSensor.setPulseAmplitudeGreen(220);
    particleSensor.setPulseAmplitudeBlue(220);
    
    // Délai pour stabilisation
    delay(100);
    
    // Nettoyer le FIFO après réveil
    int discardedSamples = 0;
    while (particleSensor.available()) {
        particleSensor.getFIFOGreen();
        particleSensor.getFIFOIR();
        particleSensor.getFIFORed();
        particleSensor.getFIFOBlue();
        particleSensor.nextSample();
        discardedSamples++;
    }
    
    if (discardedSamples > 0) {
        Serial.printf("🔋 FIFO nettoyé après réveil: %d échantillons\n", discardedSamples);
    }
    
    ppgPowered = true;
    Serial.println("✅ PPG rallumé et stabilisé");
}

// 🔋 NOUVEAU: Vérifier l'état d'alimentation
bool SensorManager::isPPGPowered() {
    return ppgPowered;
}

void SensorManager::checkPPGSensor() {
    if (ppgPowered) {
        particleSensor.check();
    }
}

void SensorManager::updatePPGSensor() {
    if (ppgPowered) {
        particleSensor.check();
    }
}

void SensorManager::readPPGData(uint32_t &green, uint32_t &ir, uint32_t &red, uint32_t &blue) {
    // Initialiser à zéro
    green = 0; ir = 0; red = 0; blue = 0;
    
    // NOUVEAU: Vérifier si PPG est alimenté
    if (!ppgPowered) {
        Serial.println("⚠️ PPG éteint - Pas de données disponibles");
        return;
    }
    
    if (particleSensor.available()) {
        // Si plusieurs échantillons sont disponibles, lire tous sauf le dernier
        while (particleSensor.available() > 1) {
            particleSensor.getFIFOGreen();
            particleSensor.getFIFOIR();
            particleSensor.getFIFORed();
            particleSensor.getFIFOBlue();
            particleSensor.nextSample();
        }
        
        // Lire le dernier échantillon disponible
        green = particleSensor.getFIFOGreen();
        ir = particleSensor.getFIFOIR();
        red = particleSensor.getFIFORed();
        blue = particleSensor.getFIFOBlue();
        particleSensor.nextSample();
    }
}

uint8_t SensorManager::readLIS3DHRegister(uint8_t reg) {
    if (!SPIManager::isSharedMode()) {
        return 0;
    }
    
    digitalWrite(SD_CS, HIGH);
    digitalWrite(LIS3DH_CS, HIGH);
    delayMicroseconds(50);
    
    digitalWrite(LIS3DH_CS, LOW);
    delayMicroseconds(50);
    
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
    SPI.transfer(0x80 | reg);
    uint8_t value = SPI.transfer(0x00);
    SPI.endTransaction();
    
    digitalWrite(LIS3DH_CS, HIGH);
    delayMicroseconds(50);
    
    return value;
}

void SensorManager::writeLIS3DHRegister(uint8_t reg, uint8_t value) {
    if (!SPIManager::isSharedMode()) {
        return;
    }
    
    digitalWrite(SD_CS, HIGH);
    digitalWrite(LIS3DH_CS, HIGH);
    delayMicroseconds(50);
    
    digitalWrite(LIS3DH_CS, LOW);
    delayMicroseconds(50);
    
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
    SPI.transfer(reg);
    SPI.transfer(value);
    SPI.endTransaction();
    
    digitalWrite(LIS3DH_CS, HIGH);
    delayMicroseconds(50);
}

bool SensorManager::readAccelerometerFast(float &x, float &y, float &z) {
    if (!SPIManager::isSharedMode()) {
        x = y = z = 0.0;
        return false;
    }
    
    // OPTIMISATION 1: Délais réduits
    digitalWrite(SD_CS, HIGH);
    digitalWrite(LIS3DH_CS, HIGH);
    delayMicroseconds(5);   // Réduit de 50 à 5 µs ✅
    
    digitalWrite(LIS3DH_CS, LOW);
    delayMicroseconds(5);   // Réduit de 50 à 5 µs ✅
    
    // OPTIMISATION 2: Fréquence SPI plus élevée
    SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));  // 8MHz au lieu de 1MHz ✅
    
    // OPTIMISATION 3: Lecture en BURST (6 registres d'un coup)
    SPI.transfer(0x80 | 0x40 | 0x28);  // Read + Auto-increment + Start à X_LOW
    uint8_t data[6];
    for (int i = 0; i < 6; i++) {
        data[i] = SPI.transfer(0x00);
    }
    
    SPI.endTransaction();
    digitalWrite(LIS3DH_CS, HIGH);
    delayMicroseconds(5);   // Réduit de 50 à 5 µs ✅
    
    // Reconstruction des valeurs (facteur corrigé)
    int16_t rawX = (int16_t)((data[1] << 8) | data[0]);
    int16_t rawY = (int16_t)((data[3] << 8) | data[2]);
    int16_t rawZ = (int16_t)((data[5] << 8) | data[4]);
    
    float scale = (4.0 / 65536.0) * 9.81;  // Facteur corrigé
    x = rawX * scale;
    y = rawY * scale;
    z = rawZ * scale;
    
    return true;
}

bool SensorManager::readAccelerometer(float &x, float &y, float &z) {
    if (!SPIManager::isSharedMode()) {
        x = y = z = 0.0;
        return false;
    }
    
    uint8_t xl = readLIS3DHRegister(0x28);
    uint8_t xh = readLIS3DHRegister(0x29);
    uint8_t yl = readLIS3DHRegister(0x2A);
    uint8_t yh = readLIS3DHRegister(0x2B);
    uint8_t zl = readLIS3DHRegister(0x2C);
    uint8_t zh = readLIS3DHRegister(0x2D);
    
    int16_t rawX = (int16_t)((xh << 8) | xl);
    int16_t rawY = (int16_t)((yh << 8) | yl);
    int16_t rawZ = (int16_t)((zh << 8) | zl);
    
    float scale = (4.0 / 65536.0) * 9.81;  // ≈ 0.0006 m/s²/LSB
    x = rawX * scale;
    y = rawY * scale;
    z = rawZ * scale;
    
    return true;
}