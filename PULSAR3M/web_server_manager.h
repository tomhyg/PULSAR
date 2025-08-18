// web_server_manager.h
#ifndef WEB_SERVER_MANAGER_H
#define WEB_SERVER_MANAGER_H

#include "config.h"

class WebServerManager {
public:
    static void setup();
    static void handleClient();
    static void stop();
    
private:
    static WebServer server;
    static void setupRoutes();
    static void handleRoot();
    static void handleConfig();
    static void handleSave();
    static void handleStart();
    static void handleBattery();          // 🔋 NOUVEAU: Déclaration manquante
    static void handleFiles();
    static void handleDownload();
    static void handlePreview();
    static void handleDelete();
    static void handleDeleteAll();
    static void handleSDDebug();
};

#endif