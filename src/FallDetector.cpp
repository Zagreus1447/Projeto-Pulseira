#include "FallDetector.h" 
#include <cmath> 

FallDetector::FallDetector(Adafruit_MPU6050 &mpu_sensor) : mpu(mpu_sensor) {
  currentState = STATE_NORMAL;
  freefallTimestamp = 0;
  // A variável 'impactTimestamp' não é mais necessária
}
void FallDetector::reset() {
  currentState = STATE_NORMAL;
  Serial.println("Detector resetado. Estado: STATE_NORMAL");
}

/**
 * @brief Implementação da nova função de status
 */
int FallDetector::getFallStatus() {
  if (currentState == STATE_FALL_CONFIRMED) {
    return 1; // 1 = Queda Confirmada
  } else {
    return 0; // 0 = Normal
  }
}

void FallDetector::update() {
  // Se a queda foi confirmada, trava aqui.
  if (currentState == STATE_FALL_CONFIRMED) return; 
  
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  float x = a.acceleration.x; float y = a.acceleration.y; float z = a.acceleration.z;
  float totalAccel_ms2 = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
  float totalAccel_g = totalAccel_ms2 / 9.81;
  
  switch (currentState) {
    case STATE_NORMAL:
      if (totalAccel_g < FREEFALL_THRESHOLD_LOW) {
        Serial.println("---------------------------------");
        Serial.println("FASE 1: Queda Livre Detectada! (Procurando 'solavanco'...)");
        currentState = STATE_FREEFALL_DETECTED;
        freefallTimestamp = millis(); 
      }
      break;
    
    // <<< ESTA É A NOVA LÓGICA DE 2 ESTÁGIOS >>>
    case STATE_FREEFALL_DETECTED:
      // 1. O 'solavanco' (Impacto) aconteceu?
      if (totalAccel_g > IMPACT_THRESHOLD_HIGH) {
        
        Serial.println("FASE 2: Impacto ('solavanco') Detectado!");
        Serial.println("!!! ALERTA DE QUEDA CONFIRMADA !!! (Enviando '1')");
        
        // Pula direto para "Confirmado" (sem checar imobilidade)
        currentState = STATE_FALL_CONFIRMED; 
      }
      
      // 2. O 'solavanco' não aconteceu a tempo? (Alarme Falso)
      if (millis() - freefallTimestamp > IMPACT_TIME_WINDOW) {
        currentState = STATE_NORMAL; // Volta ao normal
      }
      break;

    // A FASE 3 (IMPACT_DETECTED) FOI REMOVIDA
    case STATE_IMPACT_DETECTED: 
      // Esta fase não é mais usada, reseta por segurança
      currentState = STATE_NORMAL;
      break; 
    
    case STATE_FALL_CONFIRMED:
      // Trava aqui até o main() resetar
      break;
  }
}