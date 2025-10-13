#include "FallDetector.h"
#include "Arduino.h" 

FallDetector::FallDetector() {
    mpu = new Adafruit_MPU6050(); 
    
    // Inicializa variáveis de estado
    first_reading = true;
    freefall_detected = false;
    filtered_accel = 0.0;
}

void FallDetector::setup() {
    Wire.begin();

    if (!mpu->begin()) {
        Serial.println("Falha ao encontrar o chip MPU6050");
        while (1) delay(10);
    }
    Serial.println("MPU6050 Encontrado!");

    mpu->setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu->setGyroRange(MPU6050_RANGE_500_DEG);
    mpu->setFilterBandwidth(MPU6050_BAND_21_HZ);

    delay(100);
}

void FallDetector::update() {
    checkForFall();
}

void FallDetector::checkForFall() {
    sensors_event_t a, g, temp;
    mpu->getEvent(&a, &g, &temp);

    // 1. Calcular Vetor de Aceleração Total (Bruto)
    float total_accel_raw = sqrt(pow(a.acceleration.x, 2) + pow(a.acceleration.y, 2) + pow(a.acceleration.z, 2));

    // 2. Aplicar Filtro Passa-Baixa
    if (first_reading) {
        filtered_accel = total_accel_raw;
        first_reading = false;
    } else {
        filtered_accel = (ALPHA * total_accel_raw) + ((1.0 - ALPHA) * filtered_accel);
    }

    // Imprime para depuração
    //Serial.print("Aceleração Filtrada: ");
    //Serial.println(filtered_accel);

    // 3. Lógica de Detecção de Queda
    if (!freefall_detected && filtered_accel < FALL_THRESHOLD_LOW) {
        freefall_detected = true;
        freefall_start_time = millis();
        Serial.println("--- Queda Livre Detectada ---");
    }

    if (freefall_detected) {
        if (filtered_accel > FALL_THRESHOLD_HIGH) {
            Serial.println("!!!!!!!!!!!!!!!!!!!!!!");
            Serial.println("!!!   QUEDA DETECTADA  !!!");
            Serial.println("!!!!!!!!!!!!!!!!!!!!!!");
            
            freefall_detected = false;
            delay(2000);
        } else if (millis() - freefall_start_time > FALL_TIME_WINDOW) {
            Serial.println("--- Falso Alarme (timeout) ---");
            freefall_detected = false;
        }
    }
}