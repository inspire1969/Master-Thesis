#include <Arduino_LSM9DS1.h>
#include "NRF52_MBED_TimerInterrupt.h"
#include "NRF52_MBED_ISR_Timer.h"

#define RED                       22
float x, y, z;
bool Flag;



NRF52_MBED_Timer ITimer(NRF_TIMER_3);
NRF52_MBED_ISRTimer ISR_Timer;
#define TIMER_INTERVAL_14_28MS             14.28

void TimerHandler() {
  ISR_Timer.run();
}
unsigned long startTime = millis();
int ledState = 0;
void accelerometerISR() {
 Flag = true;
 ledState = !ledState;
 digitalWrite(RED, ledState);

}



void setup() {
  Serial.begin(115200);
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }
  pinMode(RED, OUTPUT);
  Serial.print("Accelerometer sample rate = ");
  Serial.print(IMU.accelerationSampleRate());
  Serial.println(" Hz");
  Serial.println();
  Serial.println("Acceleration in G's");
  Serial.println("X\tY\tZ");

  ITimer.attachInterrupt(TIMER_INTERVAL_14_28MS ,TimerHandler);
  ISR_Timer.setInterval( TIMER_INTERVAL_14_28MS, accelerometerISR);
}

void loop() {
  if(Flag == true)
  {
  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(x, y, z);
    unsigned long currentTime = millis();
    Serial.print("Time: ");
    Serial.print(currentTime - startTime);
    Serial.print(" ms, ");
    Serial.print("\t ");
    Serial.print(x*100);
    Serial.print("\t ");
    Serial.print(y*100);
    Serial.print("\t ");
    Serial.println(z*100);
    Flag = false;
    startTime = currentTime;
}
}
}
