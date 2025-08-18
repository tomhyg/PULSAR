// battery_manager.h - Gestionnaire de batterie avec MAX1704x (sans redéfinition)
#ifndef BATTERY_MANAGER_H
#define BATTERY_MANAGER_H

#include "config.h"
#include <SparkFun_MAX1704x_Fuel_Gauge_Arduino_Library.h>

// Les structures BatteryInfo et BatteryState sont définies dans config.h
// Pas de redéfinition ici

class BatteryManager {
public:
    static bool init();
    static BatteryInfo getBatteryInfo();
    static BatteryState getBatteryState();
    static float getVoltage();
    static float getPercentage();
    static float getChangeRate();
    static bool isCharging();
    static String getBatteryStatusText();
    static void printBatteryInfo();
    static void enableLowBatteryAlert(float threshold = 15.0);
    static void disableLowBatteryAlert();
    static bool isLowBatteryAlertEnabled();
    static void checkBatteryAlert();
    static void sleep();
    static void wake();
    static void quickStart();
    
    // Fonctions pour intégration AWS
    static String getBatteryDataForAWS();
    static void addBatteryDataToPayload(String& payload);
    
private:
    static SFE_MAX1704X fuelGauge;
    static bool initialized;
    static bool lowBatteryAlertEnabled;
    static float lowBatteryThreshold;
    static unsigned long lastBatteryCheck;
    static BatteryInfo lastBatteryInfo;
    static BatteryState currentState;
    
    static BatteryState calculateBatteryState(float percentage, float changeRate);
    static String stateToString(BatteryState state);
    static void updateBatteryInfo();
};

#endif