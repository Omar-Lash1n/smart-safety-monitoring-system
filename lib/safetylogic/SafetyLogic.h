#ifndef SAFETY_LOGIC_H
#define SAFETY_LOGIC_H

// ==========================================
// Hardware Abstraction (Mocking Setup)
// ==========================================
#ifdef ARDUINO
  #include <Arduino.h>
#else
  #include <stdint.h>
  #define HIGH 0x1
  #define LOW  0x0
  
  // Variables to remember what the fake function did
  extern int lastWrittenPin;
  extern int lastWrittenState;
  
  // The fake function signature
  void digitalWrite(uint8_t pin, uint8_t val);
#endif

// ==========================================
// Logic Functions
// ==========================================
int calculateThreshold(int potValue);
bool evaluateDanger(float distance, int irState, int threshold);

// دالة جديدة للتحكم في الـ LED قابلة للاختبار
void controlAlertLED(bool danger, uint8_t ledPin);

#endif