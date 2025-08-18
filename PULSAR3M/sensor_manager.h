// sensor_manager.h - Avec gestion d'alimentation PPG
#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include "config.h"

class SensorManager {
public:
    static bool initPPGSensor();
    static bool readAccelerometer(float &x, float &y, float &z);
    static void readPPGData(uint32_t &green, uint32_t &ir, uint32_t &red, uint32_t &blue);
    static void checkPPGSensor();
    static void updatePPGSensor();
    static bool readAccelerometerFast(float &x, float &y, float &z);
    
    // 🔋 NOUVEAU: Fonctions de gestion d'alimentation PPG
    static void powerDownPPG();    // Éteindre le PPG
    static void powerUpPPG();      // Rallumer le PPG
    static bool isPPGPowered();    // Vérifier l'état d'alimentation
    
private:
    static MAX86916 particleSensor;
    static bool ppgPowered;        // NOUVEAU: État d'alimentation PPG
    static uint8_t readLIS3DHRegister(uint8_t reg);
    static void writeLIS3DHRegister(uint8_t reg, uint8_t value);
};

#endif