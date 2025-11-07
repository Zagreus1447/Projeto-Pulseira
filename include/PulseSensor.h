#ifndef PULSESENSOR_H
#define PULSESENSOR_H
#include <Arduino.h>
void setupPulseSensor();
bool isHeartbeatAvailable();
int getLatestBPM();
#endif 