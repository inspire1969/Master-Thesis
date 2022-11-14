#include <Arduino_LSM9DS1.h>


/*Global Variables*/
//a counter to limit the number of cycles where values are displayed
int counter = 0; 


/*Setup Function*/
void setup()
{
    // Serial monitor setup
    Serial.begin(9600);
    while(!Serial);

     if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  // print the header
  Serial.println("aX\taY\taZ");
}



/*Main loop*/
void loop()
{          
  // Initialize Variables
  float aX, aY, aZ;

  // Check if the accelerometer is ready and if the loop has only
  //   been run less than 200 times (=~3 seconds displayed) 
  if (IMU.accelerationAvailable() & counter < 210)
  {
    // Read the accelerometer
    IMU.readAcceleration(aX, aY, aZ);
    // Scale up the values to better distinguish movements
    aX = (aX*100);
    aY =  (aY*100);
    aZ =  (aZ*100);

    // Print the values to the Serial Monitor
      Serial.print(aX);
      Serial.print('\t');
      Serial.print(aY);
      Serial.print('\t');
      Serial.print(aZ);
    
  // When the loop has run 200 times, reset the counter and delay
  //   3 seconds, then print empty lines and the new datapoint sequence
  } else if (counter >= 210) {
    counter = 0;
    delay(1428);
    Serial.println("\n\n-,-,-");
  }
  // Increment the counter and delay .01 seconds
  counter += 1;
  delay(10);
}
