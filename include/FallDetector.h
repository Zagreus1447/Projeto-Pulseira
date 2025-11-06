#ifndef FALLDETECTOR_H
#define FALLDETECTOR_H

#include <Arduino.h>
#include <Adafruit_MPU6050.h>

// --- Parâmetros de Detecção (Mova-os para cá) ---
#define FREEFALL_THRESHOLD_LOW 0.35    // (em 'g')
#define IMPACT_THRESHOLD_HIGH 3.5   // (em 'g')
#define IMPACT_TIME_WINDOW 1000   // (ms)
#define IMMOBILITY_THRESHOLD 0.2    // (em 'g')
#define IMMOBILITY_TIME 5000    // (ms)

// Enumeração dos estados de queda (definida aqui)
enum FallState {
  STATE_NORMAL,
  STATE_FREEFALL_DETECTED,
  STATE_IMPACT_DETECTED,
  STATE_FALL_CONFIRMED
};


// Definição da "Classe" (o nosso objeto detector)
class FallDetector {
public:
  // Construtor: Recebe uma "referência" ao sensor MPU
  FallDetector(Adafruit_MPU6050 &mpu_sensor);

  // Função principal, para ser chamada em loop()
  void update();

  // Função para checar se uma queda foi confirmada
  bool isFallConfirmed();

  // Função para resetar o estado (ex: após o alerta ser enviado)
  void reset();

private:
  // Variáveis internas que a classe usa
  Adafruit_MPU6050 &mpu; // Referência ao nosso sensor
  FallState currentState;
  
  unsigned long freefallTimestamp;
  unsigned long impactTimestamp;
  float lastAccelX, lastAccelY, lastAccelZ;
};

#endif // FALLDETECTOR_H