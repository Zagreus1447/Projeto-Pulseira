#include "FallDetector.h" // Inclui nosso próprio header
#include <cmath> // Adicionado por segurança para as funções sqrt() e pow()

// Implementação do Construtor
// (O : mpu(mpu_sensor) é como inicializamos a referência)
FallDetector::FallDetector(Adafruit_MPU6050 &mpu_sensor) : mpu(mpu_sensor) {
  // Inicializa o estado quando o objeto é criado
  currentState = STATE_NORMAL;
  freefallTimestamp = 0;
  impactTimestamp = 0;
}

// Implementação da função de reset
void FallDetector::reset() {
  currentState = STATE_NORMAL;
  Serial.println("Detector resetado. Estado: STATE_NORMAL");
  Serial.println("Procurando por queda livre...");
}

// Implementação da função de checagem
bool FallDetector::isFallConfirmed() {
  return (currentState == STATE_FALL_CONFIRMED);
}

// Implementação da função principal de atualização
void FallDetector::update() {
  
  // 1. Não faz nada se a queda já foi confirmada (esperando um reset)
  if (currentState == STATE_FALL_CONFIRMED) {
    return;
  }

  // 2. Lê os dados do sensor
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float x = a.acceleration.x;
  float y = a.acceleration.y;
  float z = a.acceleration.z;

  // 3. Calcula a aceleração total em 'g'
  float totalAccel_ms2 = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
  float totalAccel_g = totalAccel_ms2 / 9.81;

  // 4. Processa a Lógica de Detecção (o mesmo switch de antes)
  switch (currentState) {
    
    case STATE_NORMAL:
      // FASE 1: Procurando por Queda Livre
      if (totalAccel_g < FREEFALL_THRESHOLD_LOW) {
        Serial.println("---------------------------------");
        Serial.print("FASE 1: Queda Livre Detectada! (");
        Serial.print(totalAccel_g, 2); Serial.println("g)");
        
        currentState = STATE_FREEFALL_DETECTED;
        freefallTimestamp = millis(); 
        Serial.println("Estado: STATE_FREEFALL_DETECTED");
        Serial.println("Procurando por impacto (Acel > 3.5g)...");
      }
      break;

    case STATE_FREEFALL_DETECTED:
      // FASE 2: Procurando por Impacto
      if (totalAccel_g > IMPACT_THRESHOLD_HIGH) {
        Serial.print("FASE 2: Impacto Detectado! (");
        Serial.print(totalAccel_g, 2); Serial.println("g)");
        
        currentState = STATE_IMPACT_DETECTED;
        impactTimestamp = millis(); // Marca o tempo
        
        lastAccelX = x; lastAccelY = y; lastAccelZ = z; // Salva pos. inicial
        
        Serial.println("Estado: STATE_IMPACT_DETECTED");
        Serial.println("Procurando por imobilidade (por 5 seg)...");
      }
      
      // Checa timeout (alarme falso)
      if (millis() - freefallTimestamp > IMPACT_TIME_WINDOW) {
        Serial.println("Alarme Falso (só queda livre, sem impacto). Resetando.");
        reset(); // Chama nossa própria função de reset
      }
      break;

    // AQUI ESTÁ A CORREÇÃO
    case STATE_IMPACT_DETECTED: { // <<< CHAVE ADICIONADA AQUI

      // FASE 3: Procurando por Imobilidade
      float deltaAccel_ms2 = sqrt(pow(x - lastAccelX, 2) + pow(y - lastAccelY, 2) + pow(z - lastAccelZ, 2));
      float deltaAccel_g = deltaAccel_ms2 / 9.81;

      // Se a pessoa se mexeu
      if (deltaAccel_g > IMMOBILITY_THRESHOLD) {
        Serial.println("Alarme Falso (movimento detectado após impacto). Resetando.");
        reset(); // Chama nossa própria função de reset
      }
      
      // Se passou o tempo E NÃO se mexeu
      if (millis() - impactTimestamp > IMMOBILITY_TIME) {
        Serial.println("FASE 3: Imobilidade Confirmada.");
        Serial.println("!!! ALERTA DE QUEDA CONFIRMADA !!!");
        
        currentState = STATE_FALL_CONFIRMED; // Manda o sinal!
        Serial.println("Estado: STATE_FALL_CONFIRMED");
      }
      
      // Atualiza a "última aceleração"
      lastAccelX = x; lastAccelY = y; lastAccelZ = z;
      
      break;

    } // <<< E A CHAVE CORRESPONDENTE ADICIONADA AQUI

    case STATE_FALL_CONFIRMED:
      // Não faz nada, já foi tratado no início da função
      break;
  }
}