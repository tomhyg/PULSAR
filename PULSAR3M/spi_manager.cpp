// spi_manager.cpp
#include "spi_manager.h"

SPIMode SPIManager::currentMode = SPI_CONFIG_SD_ONLY;
bool SPIManager::accelerometerInitialized = false;

void SPIManager::initializeConfigMode() {
    Serial.println("=== INITIALISATION SPI MODE CONFIG (SD SEULE) ===");
    
    currentMode = SPI_CONFIG_SD_ONLY;
    
    // S'assurer que l'accéléromètre est déconnecté
    shutdownAccelerometer();
    
    // Configurer les pins pour SD uniquement
    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH);
    
    // Désactiver complètement le LIS3DH
    pinMode(LIS3DH_CS, OUTPUT);
    digitalWrite(LIS3DH_CS, HIGH);
    
    // Initialiser SPI pour SD uniquement
    SPI.begin(SD_CLK, SD_MISO, SD_MOSI);
    
    Serial.println("✅ Mode SPI CONFIG: SD seule sur le bus");
}

void SPIManager::initializeSharedMode() {
    Serial.println("=== INITIALISATION SPI MODE PARTAGE (SD + LIS3DH) ===");
    
    currentMode = SPI_SHARED_MODE;
    
    // Configurer les pins pour les deux périphériques
    pinMode(SD_CS, OUTPUT);
    pinMode(LIS3DH_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH);
    digitalWrite(LIS3DH_CS, HIGH);
    
    // SPI déjà initialisé
    SPI.begin(SD_CLK, SD_MISO, SD_MOSI);
    
    // Configuration manuelle du LIS3DH
    delay(100);
    
    // Test WHO_AM_I
    digitalWrite(SD_CS, HIGH);
    digitalWrite(LIS3DH_CS, HIGH);
    delayMicroseconds(50);
    
    digitalWrite(LIS3DH_CS, LOW);
    delayMicroseconds(50);
    
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
    SPI.transfer(0x80 | 0x0F); // Read WHO_AM_I
    uint8_t whoami = SPI.transfer(0x00);
    SPI.endTransaction();
    
    digitalWrite(LIS3DH_CS, HIGH);
    delayMicroseconds(50);
    
    if (whoami != 0x33) {
        Serial.println("❌ LIS3DH non détecté (WHO_AM_I = 0x" + String(whoami, HEX) + ")");
        accelerometerInitialized = false;
        return;
    }
    
    // Configuration LIS3DH
    // Reset
    digitalWrite(LIS3DH_CS, LOW);
    delayMicroseconds(50);
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
    SPI.transfer(0x20); // CTRL_REG1
    SPI.transfer(0x00); // Power down
    SPI.endTransaction();
    digitalWrite(LIS3DH_CS, HIGH);
    delay(100);
    
    // Configure
    digitalWrite(LIS3DH_CS, LOW);
    delayMicroseconds(50);
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
    SPI.transfer(0x20); // CTRL_REG1
    SPI.transfer(0x57); // 100Hz, normal mode, all axes enabled
    SPI.endTransaction();
    digitalWrite(LIS3DH_CS, HIGH);
    delay(50);
    
    // Set resolution
    digitalWrite(LIS3DH_CS, LOW);
    delayMicroseconds(50);
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
    SPI.transfer(0x23); // CTRL_REG4
    SPI.transfer(0x08); // High resolution, ±2g
    SPI.endTransaction();
    digitalWrite(LIS3DH_CS, HIGH);
    delay(50);
    
    accelerometerInitialized = true;
    Serial.println("✅ Mode SPI PARTAGE: SD + LIS3DH configuré");
}

void SPIManager::shutdownAccelerometer() {
    if (accelerometerInitialized) {
        Serial.println("🔌 Arrêt de l'accéléromètre...");
        
        digitalWrite(SD_CS, HIGH);
        delay(10);
        
        digitalWrite(LIS3DH_CS, LOW);
        delayMicroseconds(100);
        
        SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
        SPI.transfer(0x20); // CTRL_REG1
        SPI.transfer(0x00); // Power-down
        SPI.endTransaction();
        
        digitalWrite(LIS3DH_CS, HIGH);
        delayMicroseconds(100);
        
        accelerometerInitialized = false;
        Serial.println("✅ Accéléromètre arrêté");
    }
    
    pinMode(LIS3DH_CS, OUTPUT);
    digitalWrite(LIS3DH_CS, HIGH);
    delay(50);
}

bool SPIManager::isSharedMode() {
    return (currentMode == SPI_SHARED_MODE) && accelerometerInitialized;
}

SPIMode SPIManager::getCurrentMode() {
    return currentMode;
}