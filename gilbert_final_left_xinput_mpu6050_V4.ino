// Left Controller Code (I2C Master and XInput)
//I2C section created by https://www.twitch.tv/gilbertsgadgets and
//Special shoutout to GrayArtificer for I2C foundation
//gesture control base from https://github.com/shveytank/MPU_6050_Guesture_codes/blob/master/MPU6050_XYZacc3.ino
/*
    Project     Arduino XInput Library
    @author     David Madison
    @link       github.com/dmadison/ArduinoXInput
    @license    MIT - Copyright (c) 2019 David Madison

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.

    Example:      GamepadPins
    Description:  Uses all of the available pin inputs to build a 'complete'
                  Xbox gamepad, with both analog joysticks, both triggers,
                  and all of the main buttons.

 *                * Joysticks should be your typical 10k dual potentiometers.
 *                * Triggers can be either analog (pots) or digital (buttons).
                    Set the 'TriggerButtons' variable to change between the two.
 *                * Buttons use the internal pull-ups and should be connected
                    directly to ground.

                  These pins are designed around the Leonardo's layout. You
                  may need to change the pin numbers if you're using a
                  different board type

*/

#include <XInput.h>
#include <Wire.h>
#include "I2Cdev.h"
#include "MPU6050.h"
byte x = 0;

MPU6050 mpu (0x68);
MPU6050 mpuRight (0x69);

int16_t ax, ay, az;
int16_t gx, gy, gz;

int16_t ax1, ay1, az1;
int16_t gx1, gy1, gz1;

// Setup
const boolean UseLeftJoystick   = true;  // set to true to enable left joystick
const boolean InvertLeftYAxis   = true;  // set to true to use inverted left joy Y

const boolean UseRightJoystick  = true;  // set to true to enable right joystick
const boolean InvertRightYAxis  = true;  // set to true to use inverted right joy Y

const boolean UseTriggerButtons = true;   // set to false if using analog triggers

const int ADC_Max = 1023;  // 10 bit
const int ADC_Max_Trigger = 900;

// Joystick Pins
const int Pin_LeftJoyX  = A0;
const int Pin_LeftJoyY  = A1;

// Trigger Pins
const int Pin_TriggerL = A2;

const int Pin_ButtonLB = 5;

const int Pin_ButtonBack = 6;

const int Pin_ButtonL3 = 9;

// Directional Pad Pins
const int Pin_DpadUp    = 10;
const int Pin_DpadDown  = 16;
const int Pin_DpadLeft  = 14;
const int Pin_DpadRight = 15;

/********* Data packet format *************/

typedef struct {
  boolean buttonA;
  boolean buttonB;
  boolean buttonX;
  boolean buttonY;
  boolean buttonRB;
  boolean buttonStart;
  boolean buttonR3;
  int triggerRight;
  int joyX;
  int joyY;
} RIGHT_CONTROLLER_BUTTONS;

RIGHT_CONTROLLER_BUTTONS RightButtonStates;
struct MyData {
  byte X;
  byte Y;
  byte X1;
  byte Y1;
};

MyData data;
/********************* SETUP FUNCTION ************************/

void setup() {
  //  Serial.begin(9600);
  //  Serial.println("Serial started");

  Wire.begin(); // join i2c bus (address optional for master)
  mpu.initialize();
  mpuRight.initialize();
  // If using buttons for the triggers, use internal pull-up resistors
  if (UseTriggerButtons == true) {
    pinMode(Pin_TriggerL, INPUT_PULLUP);
  }
  // If using potentiometers for the triggers, set range
  else {
    XInput.setTriggerRange(0, ADC_Max_Trigger);
  }

  // Set buttons as inputs, using internal pull-up resistors
  pinMode(Pin_ButtonLB, INPUT_PULLUP);

  pinMode(Pin_ButtonBack, INPUT_PULLUP);

  pinMode(Pin_ButtonL3, INPUT_PULLUP);

  pinMode(Pin_DpadUp, INPUT_PULLUP);
  pinMode(Pin_DpadDown, INPUT_PULLUP);
  pinMode(Pin_DpadLeft, INPUT_PULLUP);
  pinMode(Pin_DpadRight, INPUT_PULLUP);

  XInput.setJoystickRange(0, ADC_Max);  // Set joystick range to the ADC
  XInput.setAutoSend(false);  // Wait for all controls before sending

  XInput.begin();
}

/******************* LOOP FUNCTION **********************/

void loop() {
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  data.X = map(ax, -17000, 17000, 0, 255 ); // X axis data
  data.Y = map(ay, -17000, 17000, 0, 255);  // Y axis data
  mpuRight.getMotion6(&ax1, &ay1, &az1, &gx1, &gy1, &gz1);
  data.X1 = map(ax1, -17000, 17000, 0, 255 ); // X axis data
  data.Y1 = map(ay1, -17000, 17000, 0, 255);  // Y axis data

  // Get the values of the buttons from the right controller via I2C
  // Send the request
  Wire.requestFrom(0x08, sizeof(RIGHT_CONTROLLER_BUTTONS)); // request data from I2C device with address 0x08
  //Wire.requestFrom(0x68, 6);

  //  Serial.println("Request Sent");

  // Give the right controller a little time to respond (Might not be needed)
  //  delay(10);

  int i = 0;
  char* RightButtons = (char*) &RightButtonStates;
  while (Wire.available()) { // secondary may send less than requested
    if (i == sizeof(RIGHT_CONTROLLER_BUTTONS)) {
      // We're about to overflow our struct. Abandon ship!
      //      Serial.println("Too much data!");
      // Flush the buffer before we leave
      while (Wire.available()) {
        (void) Wire.read();
      }
    } else {
      RightButtons[i] = Wire.read(); // receive a byte as character
      i++; //Increment to next byte in memory
    }
  }

  //  Serial.print(i);
  //  Serial.println(" bytes of data received.");

  if (i < sizeof(RIGHT_CONTROLLER_BUTTONS)) {
    //    Serial.println("***Too little data received. Caution is advised.***");
  }

  // Assign the values received over I2C to our button variables
  boolean buttonA = RightButtonStates.buttonA;

  boolean buttonB = RightButtonStates.buttonB;
  if (data.X1 > 200 or !digitalRead(RightButtonStates.buttonB)) { //gesture : down
   delay (200);
    buttonB = true;
  }
  else {
    buttonB = false;
  }
  boolean buttonX = RightButtonStates.buttonX;
  boolean buttonY = RightButtonStates.buttonY;
  boolean buttonRB = RightButtonStates.buttonRB;
  if (data.X1 < 60 or !digitalRead(RightButtonStates.buttonRB)) { //gesture : down
    delay (200); 
    buttonRB = true;
      }
  else {
    buttonRB = false;
  }

  boolean buttonStart = RightButtonStates.buttonStart;
  boolean buttonR3 = RightButtonStates.buttonR3;

  //  Serial.println("\nRight Controller Buttons\n==============================");
  //  Serial.print("Button A: ");
  //  Serial.println(buttonA);
  //  Serial.print("Button B: ");
  //  Serial.println(buttonB);
  //  Serial.print("Button X: ");
  //  Serial.println(buttonX);
  //  Serial.print("Button Y: ");
  //  Serial.println(buttonY);
  //  Serial.print("Button RB: ");
  //  Serial.println(buttonRB);
  //  Serial.print("Button Start: ");
  //  Serial.println(buttonStart);
  //  Serial.print("Button R3: ");
  //  Serial.println(buttonR3);
  //  Serial.println("==============================\n");

  // Read pin values and store in variables
  // (Note the "!" to invert the state, because LOW = pressed)
  boolean buttonLB = !digitalRead(Pin_ButtonLB);
  if (data.X < 70 or !digitalRead(Pin_ButtonLB)) { //gesture : down
    delay (100);
    buttonLB = true;
  }
  else {
    buttonLB = false;
  }
  boolean buttonBack  = !digitalRead(Pin_ButtonBack);
  boolean buttonL3 = !digitalRead(Pin_ButtonL3);

  boolean dpadUp    = !digitalRead(Pin_DpadUp);
  boolean dpadDown  = !digitalRead(Pin_DpadDown);
  boolean dpadLeft  = !digitalRead(Pin_DpadLeft);
  boolean dpadRight = !digitalRead(Pin_DpadRight);

  /************** XInput *******************************/

  // Set XInput buttons
  XInput.setButton(BUTTON_A, buttonA);
  XInput.setButton(BUTTON_B, buttonB);
  XInput.setButton(BUTTON_X, buttonX);
  XInput.setButton(BUTTON_Y, buttonY);

  XInput.setButton(BUTTON_LB, buttonLB);
  XInput.setButton(BUTTON_RB, buttonRB);

  XInput.setButton(BUTTON_BACK, buttonBack);
  XInput.setButton(BUTTON_START, buttonStart);

  XInput.setButton(BUTTON_L3, buttonL3);
  XInput.setButton(BUTTON_R3, buttonR3);

  // Set XInput DPAD values
  XInput.setDpad(dpadUp, dpadDown, dpadLeft, dpadRight);

  // Set XInput trigger values
  if (UseTriggerButtons == true) {
    // Read trigger buttons
    boolean triggerLeft  = digitalRead(Pin_TriggerL);
    if (data.Y < 60 or digitalRead(Pin_TriggerL)) { //gesture : down
     delay (100);
      triggerLeft = true;
    }
    else {
      triggerLeft = false;
    }
    boolean triggerRight = RightButtonStates.triggerRight;
    if (data.Y1 < 50 or !digitalRead(RightButtonStates.triggerRight)) { //gesture : down
      delay (200); 
      triggerRight = true;
    }
    else {
      triggerRight = false;
    }

    // Set the triggers as if they were buttons
    XInput.setButton(TRIGGER_LEFT, triggerLeft);
    XInput.setButton(TRIGGER_RIGHT, triggerRight);
  }
  else {
    // Read trigger potentiometer values
    int triggerLeft  = analogRead(Pin_TriggerL);
    int triggerRight = RightButtonStates.triggerRight;


    // Set the trigger values as analog
    XInput.setTrigger(TRIGGER_LEFT, triggerLeft);
    XInput.setTrigger(TRIGGER_RIGHT, triggerRight);
  }

  // Set left joystick
  if (UseLeftJoystick == true) {
    int leftJoyX = analogRead(Pin_LeftJoyX);
    int leftJoyY = analogRead(Pin_LeftJoyY);

    // White lie here... most generic joysticks are typically
    // inverted by default. If the "Invert" variable is false
    // then we need to do this transformation.
    if (InvertLeftYAxis == false) {
      leftJoyY = ADC_Max - leftJoyY;
    }

    XInput.setJoystick(JOY_LEFT, leftJoyX, leftJoyY);
  }

  // Set right joystick
  if (UseRightJoystick == true) {
    int rightJoyX = RightButtonStates.joyX;
    int rightJoyY = RightButtonStates.joyY;

    if (InvertRightYAxis == false) {
      rightJoyY = ADC_Max - rightJoyY;
    }

    XInput.setJoystick(JOY_RIGHT, rightJoyX, rightJoyY);
  }

  // Send control data to the computer
  XInput.send();

  delay(50);

}
