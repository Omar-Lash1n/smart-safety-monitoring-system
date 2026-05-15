// ======================================================
// Smart Safety Monitoring System
// ESP32 + FreeRTOS + Interrupt
// ======================================================
//
// Features:
// ✅ Analog Task
// ✅ Digital Task
// ✅ Communication Task
// ✅ Processing Task
// ✅ Output Task
// ✅ Logging Task
// ✅ Queue
// ✅ Mutex
// ✅ Binary Semaphore
// ✅ Interrupt
//
// ======================================================
#include "SafetyLogic.h"
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

// ======================================================
// Pins
// ======================================================

#define TRIG_PIN 16
#define ECHO_PIN 17

#define IR_PIN 27

#define POT_PIN 34

#define LED_PIN 2

#define BUTTON_PIN 4

// ======================================================
// WiFi
// ======================================================

const char* ssid = "LastDance";

const char* password =
"KosomElEmbedded123@@";

const char* serverName =
"http://192.168.1.3:8000/data";

// ======================================================
// Queue Data Structure
// ======================================================

struct LogData {

  float distance;

  int ir;

  int pot;

  bool emergency;
};

// ======================================================
// Global Variables
// ======================================================

unsigned long lastSendTime = 0;

const unsigned long sendInterval =
  10000;

bool buttonPressed = false;

float globalDistance = 0;

int globalIR = 0;

int globalPot = 0;

volatile bool emergencyMode = false;

// ======================================================
// RTOS Objects
// ======================================================

SemaphoreHandle_t buttonSemaphore;

SemaphoreHandle_t irSemaphore;

SemaphoreHandle_t serialMutex;

QueueHandle_t logQueue;

// ======================================================
// Button ISR
// ======================================================

void IRAM_ATTR buttonISR() {

  xSemaphoreGiveFromISR(
    buttonSemaphore,
    NULL
  );
}

// ======================================================
// IR ISR
// ======================================================

void IRAM_ATTR irISR() {

  xSemaphoreGiveFromISR(
    irSemaphore,
    NULL
  );
}

// ======================================================
// Ultrasonic Function
// ======================================================

float readDistanceCM() {

  digitalWrite(TRIG_PIN, LOW);

  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);

  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  long duration =
    pulseIn(ECHO_PIN, HIGH, 30000);

  float distance =
    duration * 0.034 / 2;

  return distance;
}

// ======================================================
// Ultrasonic Task
// ======================================================

void ultrasonicTask(void *pvParameters) {

  while (1) {

    globalDistance = readDistanceCM();

    vTaskDelay(pdMS_TO_TICKS(200));
  }
}

// ======================================================
// IR Task
// ======================================================

void irTask(void *pvParameters) {

  while (1) {

    if (xSemaphoreTake(
          irSemaphore,
          portMAX_DELAY)) {

      globalIR = digitalRead(IR_PIN);

      xSemaphoreTake(
        serialMutex,
        portMAX_DELAY
      );

      Serial.print("IR INTERRUPT: ");

      Serial.println(globalIR);

      xSemaphoreGive(serialMutex);
    }
  }
}

// ======================================================
// Potentiometer Task
// ======================================================

void potentiometerTask(void *pvParameters) {

  while (1) {

    globalPot = analogRead(POT_PIN);

    vTaskDelay(pdMS_TO_TICKS(300));
  }
}

// ======================================================
// Processing Task
// ======================================================

void processingTask(void *pvParameters) {
  LogData data;
  while (1) {
    // ==========================================
    // Use our Tested Logic
    // ==========================================
    int threshold = calculateThreshold(globalPot);
    bool danger = evaluateDanger(globalDistance, globalIR, threshold);

    emergencyMode = danger;

    // ==========================================
    // Queue Send
    // ==========================================
    data.distance = globalDistance;
    data.ir = globalIR;
    data.pot = globalPot;
    data.emergency = emergencyMode;

    xQueueOverwrite(logQueue, &data);
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

// ======================================================
// Alert Task
// ======================================================

void alertTask(void *pvParameters) {
  while (1) {
    // استخدام دالة اللوجيك اللي اختبرناها
    controlAlertLED(emergencyMode, LED_PIN);

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

// ======================================================
// Logger Task
// ======================================================

void loggerTask(void *pvParameters) {

  LogData receivedData;

  bool lastEmergency = false;

  while (1) {

    if (xQueueReceive(
          logQueue,
          &receivedData,
          portMAX_DELAY)) {

      // ==========================================
      // Print only when emergency starts
      // ==========================================

      if (receivedData.emergency &&
          !lastEmergency) {

        int threshold = map(
                          receivedData.pot,
                          0,
                          4095,
                          5,
                          30
                        );

        TickType_t tick =
          xTaskGetTickCount();

        xSemaphoreTake(
          serialMutex,
          portMAX_DELAY
        );

        Serial.println(
          "\n========== EMERGENCY =========="
        );

        Serial.print("Tick Count: ");

        Serial.println(tick);

        Serial.print("Distance: ");

        Serial.print(receivedData.distance);

        Serial.println(" cm");

        Serial.print("IR Sensor: ");

        if (receivedData.emergency) {

            Serial.println(1);

        } else {

          Serial.println(0);
        }

        Serial.print("Potentiometer: ");

        Serial.println(receivedData.pot);

        Serial.print("Threshold: ");

        Serial.println(threshold);

        Serial.println(
          "STATUS: DANGER DETECTED"
        );

        Serial.println(
          "================================"
        );

        xSemaphoreGive(serialMutex);
      }

      // Save previous state
      lastEmergency = receivedData.emergency;
    }

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

// ======================================================
// Communication Task
// ======================================================

// ======================================================
// Communication Task
// ======================================================

// ======================================================
// Communication Task
// ======================================================

void communicationTask(void *pvParameters) {

  bool lastEmergency = false;

  while (1) {

    bool shouldSend = false;

    // =====================================
    // Send only when emergency starts
    // =====================================

    if (emergencyMode &&
        !lastEmergency) {

      shouldSend = true;
    }

    // =====================================
    // Button Press
    // =====================================

    if (buttonPressed) {

      shouldSend = true;
    }

    // =====================================
    // Send Data
    // =====================================

    if (shouldSend &&
        WiFi.status() == WL_CONNECTED) {

      HTTPClient http;

      http.begin(serverName);

      http.addHeader(
        "Content-Type",
        "application/json"
      );

      String jsonData = "{";

      jsonData += "\"distance\":";
      jsonData += String(globalDistance);
      jsonData += ",";

      jsonData += "\"potentiometer\":";
      jsonData += String(globalPot);
      jsonData += ",";

      jsonData += "\"emergency\":";
      jsonData += String(
                    emergencyMode ?
                    "true" :
                    "false"
                  );
      jsonData += ",";

      jsonData += "\"button\":";
      jsonData += String(
                    buttonPressed ?
                    "true" :
                    "false"
                  );

      jsonData += "}";

      int httpResponseCode =
        http.POST(jsonData);

      xSemaphoreTake(
        serialMutex,
        portMAX_DELAY
      );

      Serial.println(
        "\n===== DATA SENT ====="
      );

      Serial.println(jsonData);

      Serial.print("HTTP Response: ");

      Serial.println(httpResponseCode);

      Serial.println(
        "====================="
      );

      xSemaphoreGive(serialMutex);

      http.end();

      buttonPressed = false;
    }

    // Save Previous State
    lastEmergency = emergencyMode;

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

// ======================================================
// Button Task
// ======================================================

// ======================================================
// Button Task
// ======================================================

void buttonTask(void *pvParameters) {

  while (1) {

    if (xSemaphoreTake(
          buttonSemaphore,
          portMAX_DELAY)) {

      buttonPressed = true;

      int threshold = map(
                        globalPot,
                        0,
                        4095,
                        5,
                        30
                      );

      xSemaphoreTake(
        serialMutex,
        portMAX_DELAY
      );

      Serial.println(
        "\n===== BUTTON REPORT ====="
      );

      Serial.print("Distance: ");

      Serial.print(globalDistance);

      Serial.println(" cm");

      Serial.print("IR Sensor: ");

      if (emergencyMode) {

        Serial.println(1);

      } else {

        Serial.println(0);
      }

      Serial.print("Potentiometer: ");

      Serial.println(globalPot);

      Serial.print("Threshold: ");

      Serial.println(threshold);

      Serial.print("Emergency Mode: ");

      Serial.println(emergencyMode);

      Serial.println(
        "==========================="
      );

      xSemaphoreGive(serialMutex);
    }
  }
}

// ======================================================
// Setup
// ======================================================

void setup() {

  Serial.begin(115200);

  // ==========================================
  // Pin Modes
  // ==========================================

  pinMode(TRIG_PIN, OUTPUT);

  pinMode(ECHO_PIN, INPUT);

  pinMode(IR_PIN, INPUT);

  pinMode(POT_PIN, INPUT);

  pinMode(LED_PIN, OUTPUT);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // ==========================================
  // RTOS Objects
  // ==========================================

  buttonSemaphore =
    xSemaphoreCreateBinary();

  irSemaphore =
    xSemaphoreCreateBinary();

  serialMutex =
    xSemaphoreCreateMutex();

  logQueue = xQueueCreate(
               1,
               sizeof(LogData)
             );

  // ==========================================
  // WiFi
  // ==========================================

  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);

    Serial.print(".");
  }

  Serial.println("\nWiFi Connected!");

  Serial.print("ESP32 IP: ");

  Serial.println(WiFi.localIP());

  // ==========================================
  // Interrupts
  // ==========================================

  attachInterrupt(
    digitalPinToInterrupt(BUTTON_PIN),
    buttonISR,
    FALLING
  );

  attachInterrupt(
    digitalPinToInterrupt(IR_PIN),
    irISR,
    CHANGE
  );

  // ==========================================
  // Create Tasks
  // ==========================================

  xTaskCreatePinnedToCore(
    ultrasonicTask,
    "Ultrasonic Task",
    2048,
    NULL,
    3,
    NULL,
    1
  );

  xTaskCreatePinnedToCore(
    irTask,
    "IR Task",
    2048,
    NULL,
    3,
    NULL,
    1
  );

  xTaskCreatePinnedToCore(
    potentiometerTask,
    "Potentiometer Task",
    2048,
    NULL,
    1,
    NULL,
    1
  );

  xTaskCreatePinnedToCore(
    processingTask,
    "Processing Task",
    4096,
    NULL,
    4,
    NULL,
    1
  );

  xTaskCreatePinnedToCore(
    alertTask,
    "Alert Task",
    2048,
    NULL,
    2,
    NULL,
    1
  );

  xTaskCreatePinnedToCore(
    loggerTask,
    "Logger Task",
    4096,
    NULL,
    1,
    NULL,
    0
  );

  xTaskCreatePinnedToCore(
    buttonTask,
    "Button Task",
    2048,
    NULL,
    3,
    NULL,
    0
  );

  xTaskCreatePinnedToCore(
    communicationTask,
    "Communication Task",
    4096,
    NULL,
    2,
    NULL,
    0
  );
}

// ======================================================
// Main Loop
// ======================================================

void loop() {

  // Empty
}