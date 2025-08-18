// spi_manager.h
#ifndef SPI_MANAGER_H
#define SPI_MANAGER_H

#include "config.h"

class SPIManager {
public:
    static void initializeConfigMode();
    static void initializeSharedMode();
    static void shutdownAccelerometer();
    static bool isSharedMode();
    static SPIMode getCurrentMode();
    
private:
    static SPIMode currentMode;
    static bool accelerometerInitialized;
};

#endif