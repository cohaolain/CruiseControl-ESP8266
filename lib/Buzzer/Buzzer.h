#pragma once
#include <Arduino.h>
extern const int buzzerPin;

void ackBeep(bool doTwice);
void ackBeep();
void actionBeep(bool isNice);
