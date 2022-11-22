#include <Arduino_LSM9DS1.h>
static const char channels = 3;
static const int frequency = 70;
volatile int samplesRead;
volatile int capture_index_position;
short sampleBuffer[210];

void setup() {
  capture_index_position=0;
    Serial.begin(115200);
    Serial.println("Started");

    pinMode(13,OUTPUT);

    if (!IMU.begin()) {
        Serial.println("Failed to initialize IMU!");
        while (1);
    }
}

void loop() {
  float x, y, z;
    if (capture_index_position > 210)
    {
    digitalWrite(13,LOW);
    // Print samples to the serial monitor or plotter
    for (int i = 0; i < 210; i++) {
      Serial.println( sampleBuffer[i]);
    }
    capture_index_position=0;
    samplesRead = 0;
    delay(1428);
    if (!IMU.begin()) {
        Serial.println("Failed to initialize IMU!");
        while(1);
    }
    delay(5000);
    capture_index_position=0;
    }
if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(x, y, z);
        Serial.print(x*100);
        Serial.print('\t');
        Serial.print(y*100);
        Serial.print('\t');
        Serial.println(z*100);
}
}
void onAccelerationdata(){
  capture_index_position = capture_index_position+210;
  samplesRead++;
}
