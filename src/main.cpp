#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include "FallDetector.h" // <<< Nosso detector de queda
#include "PulseSensor.h"  // <<< Nosso detector de pulso (manual)

// --- Pinos I2C ---
#define I2C_SDA_PIN 8
#define I2C_SCL_PIN 9

// --- OBJETOS GLOBAIS ---
Adafruit_MPU6050 mpu;
FallDetector detector(mpu);

// --- Variáveis de Estado ---
bool alertHasBeenSent = false;
int lastKnownBPM = 0; // <<< Variável para armazenar o último BPM
unsigned long lastBpmPrintTime = 0; // <<< Timer para impressão do BPM

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("--- Inicializando Detector de Quedas e Pulso ---");

  // --- Inicializa o MPU6050 (Detector de Queda) ---
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Serial.println("Tentando conectar ao MPU6050...");
  while (!mpu.begin()) {
    Serial.println("ERRO: MPU6050 não encontrado! Tentando novamente em 2s...");
    delay(2000);
  }
  Serial.println("MPU6050 conectado.");
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G); 
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.println("Detector de queda pronto.");

  // --- Inicializa o PulseSensor (Manual) ---
  setupPulseSensor(); // Chama a função do nosso arquivo pulsesensor.cpp

  Serial.println("--- Sistema Totalmente Operacional ---");
}

void loop() {
  
  // --- Bloco 1: Lógica do Detector de Queda ---
  // (Roda o mais rápido possível)
  detector.update();

  if (detector.isFallConfirmed() && !alertHasBeenSent) {
    Serial.println("-------------------------------------");
    Serial.println("MAIN: Queda confirmada! Tomando ação.");
    Serial.println("MAIN: Enviando alerta por WiFi/LoRa/GSM...");
    // (Seu código de alerta vai aqui)
    // ---------------------------------------------------
    alertHasBeenSent = true;
  }

  // --- Bloco 2: Lógica do Detector de Pulso ---
  // (Checa se o timer de fundo detectou um novo batimento)
  if (isHeartbeatAvailable()) {
    lastKnownBPM = getLatestBPM(); // <<< Atualiza o último BPM conhecido
    
    // Imprime imediatamente quando o batimento ocorre
    Serial.print(">>> Novo Batimento! BPM: "); 
    Serial.println(lastKnownBPM);
  }

  // --- Bloco 3: Impressão Constante do BPM ---
  // (Substitui a antiga linha de debug "analogRead(4)")
  unsigned long agora = millis();
  if (agora - lastBpmPrintTime > 1000) { // Imprime a cada 1 segundo
    
    // Imprime o último valor de BPM que temos
    Serial.print("BPM Atual: ");
    Serial.println(lastKnownBPM);
    
    lastBpmPrintTime = agora; // Reseta o timer de impressão
  }

  // Este delay afeta o detector de queda, mas não o PulseSensor (que está no timer)
  delay(10); 
}