#ifndef HEARTRATESPO2READER_H
#define HEARTRATESPO2READER_H

#include <Wire.h>
#include "MAX30105.h"        
#include "spo2_algorithm.h"  

class HeartRateSpO2Reader {
public:
    HeartRateSpO2Reader();
    void setup();
    void update();
    float getHeartRate();
    float getSpO2();

private:
    MAX30105 particleSensor; 

    // Buffers para armazenar as leituras para o algoritmo Maxim
    uint32_t irBuffer[100];
    uint32_t redBuffer[100];
    const int32_t bufferLength = 100;

    // Variáveis para guardar os resultados do algoritmo
    int32_t spo2;
    int8_t validSPO2;
    int32_t heartRate;
    int8_t validHeartRate;
};

#endif