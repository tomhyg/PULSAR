// led_manager.cpp - Version simplifiée sans mutex
#include "led_manager.h"

Adafruit_NeoPixel LEDManager::strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
LEDState LEDManager::currentState = LED_OFF;
unsigned long LEDManager::lastUpdate = 0;
bool LEDManager::lastLedState = false;

void LEDManager::init() {
    Serial.println("Initialisation LED...");
    strip.begin();
    strip.setBrightness(BRIGHTNESS);
    setColor(0, 0, 0);
    Serial.println("✅ LED initialisée");
}

void LEDManager::test() {
    Serial.println("Test LED rouge...");
    setColor(255, 0, 0);
    delay(500);
    Serial.println("Test LED vert...");
    setColor(0, 255, 0);
    delay(500);
    Serial.println("Test LED bleu...");
    setColor(0, 0, 255);
    delay(500);
    Serial.println("Test LED violet...");
    setColor(128, 0, 255);  // Test de la nouvelle couleur AWS
    delay(500);
    Serial.println("LED eteinte");
    setColor(0, 0, 0);
}

void LEDManager::setState(LEDState state) {
    currentState = state;
    Serial.println("LED state changed to: " + String(state));
}

void LEDManager::setColor(uint8_t r, uint8_t g, uint8_t b) {
    strip.setPixelColor(0, strip.Color(r, g, b));
    strip.show();
}

void LEDManager::update() {
    unsigned long now = millis();
    
    // Éviter updates trop fréquentes
    if (now - lastUpdate < 1000) return;  // Augmenté à 1 seconde
    lastUpdate = now;
    
    // Calcul clignotement
    bool ledOn = ((now / 1000) % 2) == 0;
    
    // Log seulement si changement d'état
    if (ledOn != lastLedState) {
        lastLedState = ledOn;
        
        switch (currentState) {
            case LED_CONFIG_AP:
                if (ledOn) {
                    setColor(255, 100, 0); // Orange
                } else {
                    setColor(0, 0, 0);
                }
                break;
                
            case LED_WIFI_CONNECT:
                Serial.print("LED_WIFI: ");
                if (ledOn) {
                    setColor(0, 255, 0); // Vert
                    Serial.println("ON");
                } else {
                    setColor(0, 0, 0);
                    Serial.println("OFF");
                }
                break;
                
            case LED_SD_RECORD:
                Serial.print("LED_SD: ");
                if (ledOn) {
                    setColor(0, 50, 255); // Bleu
                    Serial.println("ON");
                } else {
                    setColor(0, 0, 0);
                    Serial.println("OFF");
                }
                break;
                
            case LED_AWS_SEND:
                if (ledOn) {
                    setColor(128, 0, 255); // Violet/Magenta - NOUVELLE COULEUR
                } else {
                    setColor(0, 0, 0);
                }
                break;
                
            case LED_ERROR:
                Serial.println("LED_ERROR: ON");
                setColor(255, 0, 0); // Rouge fixe
                break;
                
            case LED_OFF:
            default:
                Serial.println("LED_OFF");
                setColor(0, 0, 0);
                break;
        }
    }
}