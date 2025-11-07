#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include "FallDetector.h"
#include "PulseSensor.h" 

#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>

constexpr char WIFI_SSID[] = "a10";
uint8_t broadcastAddress[] = {0xF4, 0x65, 0x0B, 0x46, 0x41, 0x30};

typedef struct struct_message {
  int id;
  float temp;           
  int hum;              
  unsigned int readingId; 
} struct_message;

// Cria os objetos
struct_message myData;
esp_now_peer_info_t peerInfo;
unsigned int readingId = 0;

//  OBJETOS GLOBAIS DO PROJETO
#define I2C_SDA_PIN 8
#define I2C_SCL_PIN 9
Adafruit_MPU6050 mpu;
FallDetector detector(mpu);

// Variáveis de Estado 
int lastKnownBPM = 0;
int lastFallStatus = 0; 
unsigned long lastSendTime = 0;
const long sendInterval = 2000; 

// FUNÇÕES DE CALLBACK 
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // Reduz o spam. Só imprime se falhar.
  if (status != ESP_NOW_SEND_SUCCESS) {
    Serial.println("Falha no Envio do Pacote");
  }
}

int32_t getWiFiChannel(const char *ssid) {
  if (int32_t n = WiFi.scanNetworks()) {
      for (uint8_t i=0; i<n; i++) {
          if (!strcmp(ssid, WiFi.SSID(i).c_str())) {
              return WiFi.channel(i);
          }
      }
  }
  return 0;
}

//SETUP 
void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println(" Inicializando Pulseira (v25 - 0 e 1) -");

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Serial.println("Conectando ao MPU6050...");
  while (!mpu.begin()) {
    Serial.println("ERRO: MPU6050 não encontrado!");
    delay(2000);
  }
  Serial.println("MPU6050 conectado.");
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G); 
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.println("Detector de queda pronto.");

  // Inicializa o PulseSensor
  setupPulseSensor(); // (Isso chama o nosso código v24 "Divide por 2")
  Serial.println("PulseSensor pronto.");

  //  Inicializa o ESP-NOW 
  Serial.println("Configurando ESP-NOW...");
  WiFi.mode(WIFI_STA);
  int32_t channel = getWiFiChannel(WIFI_SSID);
  if (channel == 0) {
    Serial.println("Rede 'a10' não encontrada. Usando canal 1.");
    channel = 1;
  }
  Serial.print("Usando Canal WiFi: ");
  Serial.println(channel);
  
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Erro ao inicializar ESP-NOW");
    return;
  }
  esp_now_register_send_cb(OnDataSent);
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = channel;  
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Falha ao adicionar peer");
    return;
  }
  Serial.println("ESP-NOW configurado. Transmissão pronta.");
  Serial.println("--- Sistema Totalmente Operacional ---");
}

void loop() {
  
  detector.update();
  
  // Pega o status (AGORA SÓ 0 ou 1)
  lastFallStatus = detector.getFallStatus();

  if (isHeartbeatAvailable()) {
    lastKnownBPM = getLatestBPM(); 
  }

  unsigned long agora = millis();
  
  // Envia a cada 2 segundos OU se o status for 1 (QUEDA CONFIRMADA)
  if (lastFallStatus == 1 || (agora - lastSendTime > sendInterval)) { 
    
    // Preenche a estrutura
    myData.id = 1; 
    myData.temp = (float)lastKnownBPM; 
    myData.hum = lastFallStatus;     
    myData.readingId = readingId++;
    
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

    // IMPRESSÃO NO SERIAL (REDUZIDA)
    if (result == ESP_OK) {
      Serial.print("Envio OK. ");
      Serial.print("BPM: ");
      Serial.print(myData.temp);
      Serial.print(", Queda: ");
      Serial.println(myData.hum);
      
      if (lastFallStatus == 1) {
        Serial.println("Alerta de Queda (1) enviado! Resetando detector...");
        detector.reset();
        lastFallStatus = 0; // Reseta o status local
      }
    }
    else {
      Serial.println("Erro ao enviar dados.");
    }
    
    lastSendTime = agora; 
  }
  
  delay(10); 
}