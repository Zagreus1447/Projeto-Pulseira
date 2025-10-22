#include <Arduino.h>
#include "FallDetector.h"
#include "HeartRateSpO2Reader.h"

FallDetector myFallDetector;
HeartRateSpO2Reader myHeartRateSpO2Reader;

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10); // Espera a porta serial conectar

    Serial.println("Iniciando sistema de monitoramento...");

    myFallDetector.setup();
    myHeartRateSpO2Reader.setup();

    Serial.println("Setup concluído. Monitorando...");
}

void loop() {
    myFallDetector.update();
    myHeartRateSpO2Reader.update();

    //Imprime os dados de SpO2 e Frequência Cardíaca
    Serial.print("Heart Rate: ");
    Serial.print(myHeartRateSpO2Reader.getHeartRate());
    Serial.print(" bpm / SpO2: ");
    Serial.print(myHeartRateSpO2Reader.getSpO2());
    Serial.println(" %");

    delay(50);
}