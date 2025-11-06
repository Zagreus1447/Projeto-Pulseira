#ifndef PULSESENSOR_H
#define PULSESENSOR_H

#include <Arduino.h>

/**
 * @brief Configura o PulseSensor e inicia o timer de hardware
 * para amostragem em segundo plano.
 */
void setupPulseSensor();

/**
 * @brief Verifica se um novo batimento foi detectado desde a última checagem.
 * @return true se houver um novo batimento, false caso contrário.
 */
bool isHeartbeatAvailable();

/**
 * @brief Obtém o último valor de BPM (Batimentos Por Minuto) calculado.
 * @return O valor de BPM (int).
 */
int getLatestBPM();

#endif // PULSESENSOR_H