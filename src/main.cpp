#include <Arduino.h>
#include "FallDetector.h"

FallDetector myFallDetector;

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10); // Espera a porta serial conectar

    Serial.println("Detector de quedas iniciando...");

    myFallDetector.setup();

    Serial.println("Setup concluído. Monitorando quedas...");
}

void loop() {
    myFallDetector.update();
    delay(50);
}