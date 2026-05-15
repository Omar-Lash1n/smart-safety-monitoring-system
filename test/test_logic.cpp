#include <unity.h>
#include "SafetyLogic.h"

// ==========================================
// Fake Hardware Implementation (Mock)
// ==========================================
int lastWrittenPin = -1;
int lastWrittenState = -1;

void digitalWrite(uint8_t pin, uint8_t val) {
    lastWrittenPin = pin;
    lastWrittenState = val;
}

// ==========================================
// Setup & Teardown
// ==========================================
void setUp(void) {
    // Reset the fake hardware before each test
    lastWrittenPin = -1;
    lastWrittenState = -1;
}

void tearDown(void) {
}

// ==========================================
// Logic Tests (REQ-01, REQ-02, REQ-03)
// ==========================================
void test_calculate_threshold_min(void) {
    TEST_ASSERT_EQUAL_INT(5, calculateThreshold(0));
}

void test_calculate_threshold_max(void) {
    TEST_ASSERT_EQUAL_INT(30, calculateThreshold(4095));
}

void test_calculate_threshold_mid(void) {
    TEST_ASSERT_EQUAL_INT(17, calculateThreshold(2047));
}

void test_evaluate_danger_ultrasonic_safe(void) {
    TEST_ASSERT_FALSE(evaluateDanger(20.0, 0, 15));
}

void test_evaluate_danger_ultrasonic_danger(void) {
    TEST_ASSERT_TRUE(evaluateDanger(10.0, 0, 15));
}

void test_evaluate_danger_ir_danger(void) {
    TEST_ASSERT_TRUE(evaluateDanger(50.0, 1, 15));
}

// ==========================================
// Hardware Mocking Tests (REQ-04)
// ==========================================
void test_control_alert_led_danger(void) {
    // Act: Send danger signal to PIN 2
    controlAlertLED(true, 2);
    
    // Assert: Check if fake digitalWrite was called correctly
    TEST_ASSERT_EQUAL_INT(2, lastWrittenPin);
    TEST_ASSERT_EQUAL_INT(HIGH, lastWrittenState);
}

void test_control_alert_led_safe(void) {
    // Act: Send safe signal to PIN 2
    controlAlertLED(false, 2);
    
    // Assert: Check if fake digitalWrite turned it off
    TEST_ASSERT_EQUAL_INT(2, lastWrittenPin);
    TEST_ASSERT_EQUAL_INT(LOW, lastWrittenState);
}

// ==========================================
// Main
// ==========================================
int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    RUN_TEST(test_calculate_threshold_min);
    RUN_TEST(test_calculate_threshold_max);
    RUN_TEST(test_calculate_threshold_mid);
    
    RUN_TEST(test_evaluate_danger_ultrasonic_safe);
    RUN_TEST(test_evaluate_danger_ultrasonic_danger);
    RUN_TEST(test_evaluate_danger_ir_danger);
    
    // Run new mock tests
    RUN_TEST(test_control_alert_led_danger);
    RUN_TEST(test_control_alert_led_safe);
    
    UNITY_END();
    return 0;
}