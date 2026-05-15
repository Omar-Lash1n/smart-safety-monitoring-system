#ifndef SAFETY_LOGIC_H
#define SAFETY_LOGIC_H

// دالة لحساب الـ Threshold بناءً على قراءة الـ Potentiometer (REQ-01)
int calculateThreshold(int potValue);

// دالة لتحديد حالة الخطر بناءً على الحساسات (REQ-02 & REQ-03)
bool evaluateDanger(float distance, int irState, int threshold);

#endif