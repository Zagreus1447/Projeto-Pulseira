#ifndef FALLDETECTOR_H
#define FALLDETECTOR_H

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <cmath>

class FallDetector {
public:
    FallDetector();
    void setup();
    void update();

private:
    // Variáveis e objetos privados 
    Adafruit_MPU6050 *mpu;

    // Parâmetros de Detecção
    const float FALL_THRESHOLD_LOW = 3.5;
    const float FALL_THRESHOLD_HIGH = 30.0;
    const int FALL_TIME_WINDOW = 500;

    // Parâmetros do Filtro
    const float ALPHA = 0.8;
    float filtered_accel = 0.0;
    bool first_reading = true;

    // Variáveis de Estado
    bool freefall_detected = false;
    unsigned long freefall_start_time = 0;

    // Funções auxiliares privadas (se necessário)
    void checkForFall();
};

#endif // FALLDETECTOR_H