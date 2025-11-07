#ifndef FALLDETECTOR_H
#define FALLDETECTOR_H
#include <Arduino.h>
#include <Adafruit_MPU6050.h>

#define FREEFALL_THRESHOLD_LOW 0.25
#define IMPACT_THRESHOLD_HIGH 3.5
#define IMPACT_TIME_WINDOW 1000
#define IMMOBILITY_THRESHOLD 0.2
#define IMMOBILITY_TIME 5000

// Nossos 3 estados de lógica
enum FallState {
  STATE_NORMAL,
  STATE_FREEFALL_DETECTED,
  STATE_IMPACT_DETECTED,
  STATE_FALL_CONFIRMED
};

class FallDetector {
public:
  FallDetector(Adafruit_MPU6050 &mpu_sensor);
  void update();
  
  /**
   * @brief Retorna o estado de queda atual.
   * 0 = Normal
   * 1 = Queda Confirmada
   */
  int getFallStatus(); 
  
  void reset();
private:
  Adafruit_MPU6050 &mpu; 
  FallState currentState;
  unsigned long freefallTimestamp;
  unsigned long impactTimestamp;
  float lastAccelX, lastAccelY, lastAccelZ;
};
#endif // FALLDETECTOR_H