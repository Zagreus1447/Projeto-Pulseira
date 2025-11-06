#include "PulseSensor.h"

// --- Configurações do PulseSensor Manual ---
#define PULSE_INPUT_PIN 4     // Pino ADC (GPIO 4) onde o sinal (S) está conectado
#define ADC_THRESHOLD 2300    // Limiar. AJUSTE ESTE VALOR (0-4095)!

// --- Variáveis do Timer de Hardware ---
hw_timer_t *timer = NULL; // Ponteiro para o timer

// --- Variáveis de Detecção de Batimento ---
// 'volatile' é crucial para variáveis usadas dentro de um ISR (timer)
volatile int latestBPM = 0;
volatile bool newBeatAvailable = false;
volatile int latestSignal = 0;

volatile unsigned long lastBeatTime = 0; // Tempo do último batimento
volatile bool pulseDetected = false;     // Flag para evitar múltiplos picos

/**
 * @brief Esta é a Rotina de Serviço de Interrupção (ISR)
 * Ela é chamada automaticamente pelo timer do ESP32 a cada 2ms (500Hz).
 * NÃO coloque 'Serial.print' ou 'delay' aqui dentro!
 */
void IRAM_ATTR onTimer() {
  // 1. Lê o valor analógico (0-4095 no ESP32-C3)
  latestSignal = analogRead(PULSE_INPUT_PIN);

  unsigned long currentTime = millis();

  // 2. Lógica de Detecção de Pico (Batimento)
  // Se o sinal passou o limiar E não estávamos já em um pulso
  if (latestSignal > ADC_THRESHOLD && !pulseDetected) {
    
    // Calcula o IBI (Intervalo Entre Batimentos)
    unsigned long ibi = currentTime - lastBeatTime;

    // Filtra ruído (IBI > 250ms = 240 BPM max)
    if (ibi > 250) { 
      latestBPM = 60000 / ibi; // Converte IBI (ms) para BPM
      lastBeatTime = currentTime;
      newBeatAvailable = true; // Avisa o loop() principal
    }
    
    pulseDetected = true; // Marca que estamos "dentro" de um pulso
  }
  // Se o sinal caiu abaixo do limiar
  else if (latestSignal < ADC_THRESHOLD) {
    pulseDetected = false; // Reseta a flag, pronto para o próximo pulso
  }
}

/**
 * @brief Implementação da função setupPulseSensor()
 */
void setupPulseSensor() {
  Serial.println("Configurando o PulseSensor (Modo Manual)...");
  pinMode(PULSE_INPUT_PIN, INPUT);

  // --- Configuração do Timer de Hardware (Timer 0) ---
  // API correta para ESP32 (e C3)
  
  // Timer 0, prescaler 80 (para 1MHz), contagem crescente
  timer = timerBegin(0, 80, true);
  
  // Anexa nossa função 'onTimer' ao timer
  timerAttachInterrupt(timer, &onTimer, true);
  
  // Define o timer para disparar a cada 2000 micro-segundos (2ms)
  // (1MHz / 2000 = 500Hz)
  timerAlarmWrite(timer, 2000, true); // true = auto-reload
  
  // Inicia o timer
  timerAlarmEnable(timer);
  
  Serial.println("PulseSensor rodando em segundo plano (Timer 0).");
}

/**
 * @brief Implementação da função isHeartbeatAvailable()
 */
bool isHeartbeatAvailable() {
  if (newBeatAvailable) {
    newBeatAvailable = false; // Reseta a flag
    return true;
  }
  return false;
}

/**
 * @brief Implementação da função getLatestBPM()
 */
int getLatestBPM() {
  return latestBPM;
}