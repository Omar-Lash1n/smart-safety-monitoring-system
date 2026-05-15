#include "SafetyLogic.h"

int calculateThreshold(int potValue) {
    // محاكاة لدالة map الخاصة بـ Arduino
    // map(globalPot, 0, 4095, 5, 30);
    if (potValue < 0) potValue = 0;
    if (potValue > 4095) potValue = 4095;
    
    return (potValue - 0) * (30 - 5) / (4095 - 0) + 5;
}

bool evaluateDanger(float distance, int irState, int threshold) {
    bool danger = false;
    
    // REQ-02: Ultrasonic Detection
    if (distance > 2.0 && distance < (float)threshold) {
        danger = true;
    }
    
    // REQ-03: IR Detection (1 means HIGH/Danger)
    if (irState == 1) {
        danger = true;
    }
    
    return danger;
}

void controlAlertLED(bool danger, uint8_t ledPin) {
    if (danger) {
        digitalWrite(ledPin, HIGH);
    } else {
        digitalWrite(ledPin, LOW);
    }
}