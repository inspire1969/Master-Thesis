#include<SD.h>
#include<SPI.h>
#include "Wire.h"
#include <Arduino_LSM9DS1.h>
#include "NRF52_MBED_TimerInterrupt.h"
#include "NRF52_MBED_ISR_Timer.h"

#define RED                       22

int chipSelect = 10;
File mySensorData;

float x, y, z;
bool Flag;

NRF52_MBED_Timer ITimer(NRF_TIMER_3);
NRF52_MBED_ISRTimer ISR_Timer;
#define TIMER_INTERVAL_14_28MS             14.28

void TimerHandler() {
  ISR_Timer.run();
}

void accelerometerISR() {
  Flag = true;
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

  ITimer.attachInterrupt(TIMER_INTERVAL_14_28MS, TimerHandler);
  ISR_Timer.setInterval(TIMER_INTERVAL_14_28MS, accelerometerISR);

  SD.begin(4);
  while (!Serial);
  Serial.println("Started");
}

void loop() {
  if(Flag == true) {
    if (IMU.accelerationAvailable()) {
      IMU.readAcceleration(x, y, z);

      mySensorData = SD.open("PTData.txt", FILE_WRITE);
      if (mySensorData) {
        Serial.print(x);
        Serial.print('\t');
        Serial.print(y);
        Serial.print('\t');
        Serial.println(z);

        mySensorData.println(x);
        mySensorData.println(y);
        mySensorData.println(z);
        mySensorData.close();
      }

      unsigned long currentTime = millis();
      Serial.print("Time: ");
      Serial.print(currentTime - startTime);
      Serial.print(" ms, ");
      Serial.print(x);
      Serial.print(", ");
      Serial.print(y);
      Serial.print(", ");
      Serial.println(z);
      Flag = false;
      startTime = currentTime;
    }
  }
}
