/*
  Callback LED

  This example creates a BLE peripheral with service that contains a
  characteristic to control an LED. The callback features of the
  library are used.

  The circuit:
  - Arduino MKR WiFi 1010, Arduino Uno WiFi Rev2 board, Arduino Nano 33 IoT,
    Arduino Nano 33 BLE, or Arduino Nano 33 BLE Sense board.

  You can use a generic BLE central app, like LightBlue (iOS and Android) or
  nRF Connect (Android), to interact with the services and characteristics
  created in this sketch.

  This example code is in the public domain.
*/

#include <ArduinoBLE.h>
#include <Arduino_LSM9DS1.h>

#include <TensorFlowLite.h>
#include <tensorflow/lite/micro/all_ops_resolver.h>
#include <tensorflow/lite/micro/micro_error_reporter.h>
#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/schema/schema_generated.h>
#include <tensorflow/lite/version.h>

/*-------------------------------------------
 neural network setup, added by Wanli 29.04.2022
-----------------------------------------------*/
#include "classification_model.h"

// sampling rate = 10 Hz, 2second*10*3 for the input of the neural network 
const int numSamples = 0;

int samplesRead = 0;

// global variables used for TensorFlow Lite (Micro)
tflite::MicroErrorReporter tflErrorReporter;

// pull in all the TFLM ops, you can remove this line and
// only pull in the TFLM ops you need, if would like to reduce
// the compiled size of the sketch.
tflite::AllOpsResolver tflOpsResolver;

const tflite::Model* tflModel = nullptr;
tflite::MicroInterpreter* tflInterpreter = nullptr;
TfLiteTensor* tflInputTensor = nullptr;
TfLiteTensor* tflOutputTensor = nullptr;

// Create a static memory buffer for TFLM, the size may need to
// be adjusted based on the model you are using
constexpr int tensorArenaSize = 10 * 1024;
byte tensorArena[tensorArenaSize] __attribute__((aligned(16)));

// array to map bike status index to a name
const char* STATUS[] = {
  "bike stopping",
  "bike moving",
  "bike falling down"
};

/*----------------------------------------------*/


int State = 0;

const int ledPin = LED_BUILTIN; // pin to use for the LED
const int ledPinGreen = 11;
const int ledPinRed = 10;
const int ledPinYellow = 9;

void toggle(int PIN){
  digitalWrite(PIN, !digitalRead(PIN)); 
}

/*bool checkThreshold(float x, float y, float z){
  if (abs(x) < 0.75 && abs(y) < 0.75 && z < 1.5 && z > 0.5){
    return false; // normal accl. behavior
  }
  else{
    return true; // abnormal accl. behavior -> trigger alarm
  }
}*/
void disableAllLed(){
  digitalWrite(ledPinGreen, LOW);
  digitalWrite(ledPinRed, LOW);
  digitalWrite(ledPinYellow, LOW);
}


void setup() {
  Serial.begin(9600);
  while (!Serial);

  // led init
  pinMode(ledPin, OUTPUT); // use the LED pin as an output
  pinMode(ledPinRed, OUTPUT);
  pinMode(ledPinGreen, OUTPUT);
  pinMode(ledPinYellow, OUTPUT);

  
  // begin BLE initialization
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");

    while (1);
  }

  //IMU Init
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  /*-------------------------------------------
 neural network interpreter, added by Wanli 29.04.2022
-----------------------------------------------*/
  // get the TFL representation of the model byte array
  tflModel = tflite::GetModel(classification_model);
  if (tflModel->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Model schema mismatch!");
    while (1);
  }

  // Create an interpreter to run the model
  tflInterpreter = new tflite::MicroInterpreter(tflModel, tflOpsResolver, tensorArena, tensorArenaSize, &tflErrorReporter);

  // Allocate memory for the model's input and output tensors
  tflInterpreter->AllocateTensors();

  // Get pointers for the model's input and output tensors
  tflInputTensor = tflInterpreter->input(0);
  tflOutputTensor = tflInterpreter->output(0);
/*----------------------------------------------*/
}



void loop() {
  switch(State){
    case 1: //locked
      toggle(ledPinYellow);
      Serial.println("Bike is locked.");
      break;

    case 2: //unlocked
      disableAllLed();
      toggle(ledPinGreen);
      Serial.println("Bike unlocked.");
      break;
      
    default:
      State = 1;
      Serial.println("Default Case // Switch to Locked");
      
    }


  // measure the acceleration data and run NN to classify the bike's status
  float x, y, z, maxValue, temp;
  int maxIndex;

  Serial.println("s1.");  
  
  samplesRead = 0;
  Serial.println("s2.");
  while (samplesRead < numSamples) {
    if (IMU.accelerationAvailable() ) {
      IMU.readAcceleration(x, y, z);
      Serial.println("s3.");
            
      Serial.print(x);
      Serial.print('\t');
      Serial.print(y);
      Serial.print('\t');
      Serial.println(z);
// read the IMU data and feed into the neural network
      tflInputTensor->data.f[samplesRead * 3 + 0] = x;
      tflInputTensor->data.f[samplesRead * 3 + 1] = y;
      tflInputTensor->data.f[samplesRead * 3 + 2] = z;

      samplesRead++;
      Serial.println(samplesRead);

      if (samplesRead == numSamples) {
        Serial.println("s4.");
        // Run inferencing
        TfLiteStatus invokeStatus = tflInterpreter->Invoke();
        Serial.println("s5.");
        if (invokeStatus != kTfLiteOk) {
          Serial.println("s6.");
          Serial.println("Invoke failed!");
          while (1);
          return;
          }
        Serial.println("s7.");
        maxIndex = 0;
        maxValue = 0;
        temp = 0;
        for (int i = 0; i <3; i++) {
          Serial.println("s8.");
          //Serial.print(STATUS[i]);
          //Serial.print(": ");
          //Serial.println(tflOutputTensor->data.f[i], 6); // 6 means number of decimal places
          temp = tflOutputTensor->data.f[i];
            // find the index of the maximum value, 0 stop; 1 moving; 2 falling
          if (temp > maxValue){
            maxValue = temp;
            maxIndex = i; // 
            }
          }
        }
      }
    }

  if (maxIndex == 0){
    Serial.println("Bike is stopped.");
    }
    
// 2 unlocked, 1 locked; 5 alarm, 4 for sending message

  if (maxIndex == 1){
    if (State == 1){
      disableAllLed();
      Serial.println("Suspicious stealing detected!");
    }
    else{
      Serial.println("Bike is moving.");  
      }
    }
  // bike is falling down, 
  // if it is in unlocked State, it will then count the number of stop using threshold algorithm  
  if (maxIndex == 2){
    if (State == 2){
      Serial.println("Suspicious accident happened!");
      }
    else{
    Serial.println("Bike is falling.");
      } 
    }
  }
