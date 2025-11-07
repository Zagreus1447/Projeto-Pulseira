#include "PulseSensor.h"
#include <Arduino.h>

#define PULSE_INPUT_PIN 4     
//  Filtros Fisiológicos 
#define REAL_BPM_MIN 45     // IBI 1333ms
#define REAL_BPM_MAX 90     // IBI 667ms
#define DOUBLE_BPM_MAX 180  // IBI 333ms (O "eco" de 114-146 está aqui)

// O Limiar Estático (CALIBRADO COM SEUS DADOS)
#define STATIC_THRESHOLD 2380  // (Ruído ~2360, Pico ~2430)

//  Variáveis do Timer de Hardware 
hw_timer_t *timer = NULL; 

//  Variáveis de Detecção de Batimento 
volatile int Signal;
volatile int IBI = 600;
volatile bool Pulse = false;
volatile bool newBeatAvailable = false;
volatile int BPM = 0;
volatile unsigned long lastBeatTime = 0; // Tempo do último batimento VÁLIDO
volatile unsigned long sampleCounter = 0; // Tempo total (em ms)

// --- Variáveis da Média Móvel (20 medidas) ---
#define BPM_HISTORY_SIZE 20 // <<< SEU PEDIDO DE 20 MEDIDAS
volatile int bpmHistory[BPM_HISTORY_SIZE];
volatile int bpmHistoryIndex = 0;
volatile int validHistoryEntries = 0;


/**
 * @brief Esta é a Rotina de Serviço de Interrupção (ISR)
 * Ela é chamada automaticamente pelo timer do ESP32 a cada 5ms (200Hz).
 */
void IRAM_ATTR onTimer() {
  Signal = analogRead(PULSE_INPUT_PIN);
  sampleCounter += 5; // Amostragem de 5ms (200Hz)
  
  unsigned long timeSinceLastBeat = sampleCounter - lastBeatTime;

  // Lógica de Detecção de Batimento (Simples) 
  
  // Se o sinal subiu acima do nosso limiar (2380) E não estávamos em um pulso
  if (Signal > STATIC_THRESHOLD && Pulse == false) {
    
    // Filtro de "Janela de Tempo" 
    // O "período cego" é o mais rápido que um humano pode ir (180 BPM)
    if (timeSinceLastBeat > 333) { // (IBI para 180 BPM)
      
      Pulse = true;
      IBI = timeSinceLastBeat;
      lastBeatTime = sampleCounter;
      
      int instantaneousBPM = 60000 / IBI;
      int finalBPM = 0;

      //  A "LÓGICA 114 = 57" 
      
      // Caso 1: O batimento é muito rápido (90-180 BPM)?
      if (instantaneousBPM > REAL_BPM_MAX && instantaneousBPM < DOUBLE_BPM_MAX) {
        // É uma "Contagem Dupla" (ex: 114). Divida por 2.
        finalBPM = instantaneousBPM / 2;
      }
      // Caso 2: O batimento está na faixa real (45-90 BPM)?
      else if (instantaneousBPM >= REAL_BPM_MIN && instantaneousBPM <= REAL_BPM_MAX) {
        // É um batimento real (ex: 75). Use-o.
        finalBPM = instantaneousBPM;
      }
      // Caso 3: É ruído (<45 ou >180).
      // 'finalBPM' continua 0 e será ignorado.

      // Média Móvel 
      // Só adiciona ao histórico se for um batimento válido
      if (finalBPM > 0) {
        bpmHistory[bpmHistoryIndex] = finalBPM;
        bpmHistoryIndex = (bpmHistoryIndex + 1) % BPM_HISTORY_SIZE;
        
        if (validHistoryEntries < BPM_HISTORY_SIZE) {
            validHistoryEntries++;
        }
        
        int totalBPM = 0;
        for (int i = 0; i < validHistoryEntries; i++) {
          totalBPM += bpmHistory[i];
        }
        BPM = totalBPM / validHistoryEntries;
        newBeatAvailable = true;
      }
    }
  }

  // Lógica de Fim de Batimento 
  if (Signal < STATIC_THRESHOLD && Pulse == true) {
    Pulse = false; // O pulso acabou, estamos prontos para o próximo
  }

  //  Lógica de Timeout (Sem Dedo)
  if (timeSinceLastBeat > 2000) { // Se 2s se passaram
    
    // Zera o histórico
    for (int i = 0; i < BPM_HISTORY_SIZE; i++) { bpmHistory[i] = 0; }
    bpmHistoryIndex = 0; validHistoryEntries = 0;
    
    if (BPM != 0) {
      BPM = 0;
      newBeatAvailable = true;
    }
  }
}

/**
 * @brief Implementação da função setupPulseSensor()
 */
void setupPulseSensor() {
  Serial.println("Configurando o PulseSensor (v24 - Lógica 'Divide por 2')...");
  pinMode(PULSE_INPUT_PIN, INPUT);
  
  // Inicializa o histórico
  for (int i = 0; i < BPM_HISTORY_SIZE; i++) {
      bpmHistory[i] = 0;
  }
  bpmHistoryIndex = 0;
  validHistoryEntries = 0;

  // Configuração do Timer de Hardware (Timer 0)
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 5000, true); 
  timerAlarmEnable(timer);
  
  Serial.println("PulseSensor rodando em segundo plano.");
}

/**
 * @brief Implementação da função isHeartbeatAvailable()
 */
bool isHeartbeatAvailable() {
  if (newBeatAvailable) {
    newBeatAvailable = false;
    return true;
  }
  return false;
}

/**
 * @brief Implementação da função getLatestBPM()
 */
int getLatestBPM() {
  return BPM;
}