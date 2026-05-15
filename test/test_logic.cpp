#include <unity.h>
#include "SafetyLogic.h"

// تعمل قبل كل اختبار
void setUp(void) {
}

// تعمل بعد كل اختبار
void tearDown(void) {
}

// اختبار REQ-01: Threshold Mapping
void test_calculate_threshold_min(void) {
    TEST_ASSERT_EQUAL_INT(5, calculateThreshold(0));
}

void test_calculate_threshold_max(void) {
    TEST_ASSERT_EQUAL_INT(30, calculateThreshold(4095));
}

void test_calculate_threshold_mid(void) {
    TEST_ASSERT_EQUAL_INT(17, calculateThreshold(2047));
}

// اختبار REQ-02: Ultrasonic Danger
void test_evaluate_danger_ultrasonic_safe(void) {
    // Distance 20, IR Safe (0), Threshold 15 -> Should be Safe (false)
    TEST_ASSERT_FALSE(evaluateDanger(20.0, 0, 15));
}

void test_evaluate_danger_ultrasonic_danger(void) {
    // Distance 10, IR Safe (0), Threshold 15 -> Should be Danger (true)
    TEST_ASSERT_TRUE(evaluateDanger(10.0, 0, 15));
}

// اختبار REQ-03: IR Danger
void test_evaluate_danger_ir_danger(void) {
    // Distance 50 (Safe), IR Danger (1), Threshold 15 -> Should be Danger (true)
    TEST_ASSERT_TRUE(evaluateDanger(50.0, 1, 15));
}

// نقطة البداية لتشغيل الاختبارات
int main(int argc, char **argv) {
    UNITY_BEGIN();
    
    RUN_TEST(test_calculate_threshold_min);
    RUN_TEST(test_calculate_threshold_max);
    RUN_TEST(test_calculate_threshold_mid);
    
    RUN_TEST(test_evaluate_danger_ultrasonic_safe);
    RUN_TEST(test_evaluate_danger_ultrasonic_danger);
    RUN_TEST(test_evaluate_danger_ir_danger);
    
    UNITY_END();
    return 0;
}