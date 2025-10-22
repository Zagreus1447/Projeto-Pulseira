// HeartRateSpO2Reader.cpp

#include "HeartRateSpO2Reader.h"
#include "Arduino.h"

HeartRateSpO2Reader::HeartRateSpO2Reader() {
    // Inicializa as variáveis
    spo2 = 0;
    validSPO2 = 0;
    heartRate = 0;
    validHeartRate = 0;
}

void HeartRateSpO2Reader::setup() {
    // Inicializa o sensor usando a biblioteca SparkFun
    if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
        Serial.println("MAX30102 não encontrado. Verifique as conexões.");
        while (1) delay(10);
    }
    Serial.println("MAX30102 Encontrado!");

    // Configurações recomendadas para SpO2
    byte ledBrightness = 0x1F; // 0-255
    byte sampleAverage = 32;  // 1, 2, 4, 8, 16, 32
    byte ledMode = 2;        // 1 = Red, 2 = Red+IR, 3 = Red+IR+Green
    int sampleRate = 100;    // 50, 100, 200, 400, 800, 1000, 1600, 3200
    int pulseWidth = 411;     // 69, 118, 215, 411
    int adcRange = 4096;      // 2048, 4096, 8192, 16384

    particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
}

void HeartRateSpO2Reader::update() {
    // Passo 1: Verificação de presença de dedo.
    // Lemos o valor IR "DC" atual. Se for muito baixo, não há dedo.
    // O valor 50000 é um limiar experimental, pode ser ajustado.
    if (particleSensor.getIR() < 50000) {
        // Dedo não detectado, reseta os valores.
        validSPO2 = 0;
        validHeartRate = 0;
        heartRate = 0;
        spo2 = 0;
        return; // Sai da função e não faz o cálculo.
    }

    // Passo 2: Se um dedo foi detectado, coleta 100 amostras.
    // Este bloco só executa se o getIR() for > 50000.
    for (int i = 0; i < bufferLength; i++) {
        // Espera até que uma nova amostra esteja disponível no FIFO do sensor
        while (particleSensor.available() == false) {
            particleSensor.check();
        }
        // Lê os valores e armazena nos buffers
        redBuffer[i] = particleSensor.getRed();
        irBuffer[i] = particleSensor.getIR();
        // Avança para a próxima amostra no FIFO
        particleSensor.nextSample();
    }

    // Passo 3: Com os buffers cheios, chama o algoritmo para o cálculo.
    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
}

float HeartRateSpO2Reader::getHeartRate() {
    // Retorna o valor calculado apenas se o algoritmo o considerou válido
    if (validHeartRate) {
        return heartRate;
    }
    return 0.0;
}

float HeartRateSpO2Reader::getSpO2() {
    // Retorna o valor calculado apenas se o algoritmo o considerou válido
    if (validSPO2) {
        // Adiciona um limite para não mostrar valores > 100%
        if (spo2 > 100) {
            return 100.0;
        }
        return spo2;
    }
    return 0.0;
}