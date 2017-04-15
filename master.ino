// Wire Master Reader
// https://www.arduino.cc/en/Tutorial/MasterReader
// by Nicholas Zambetti <http://www.zambetti.com>

// Demonstrates use of the Wire library
// Reads data from an I2C/TWI slave device
// Refer to the "Wire Slave Sender" example for use with this

// Created 29 March 2006

// This example code is in the public domain.


#include <Wire.h>

void setup() {
  Wire.begin();        // join i2c bus (address optional for master)
  Serial.begin(9600);  // start serial for output
}

void loop() {
  Wire.requestFrom(4, 4);    // request 4 bytes from slave device #4

  while (Wire.available()) { // slave may send less than requested
    byte c = Wire.read(); // receive a byte as character
    if (c < 0x10) {
      Serial.print("0");
    }
    Serial.print(c, HEX);         // print the character
    Serial.print(" ");
  }
  Serial.println();

  delay(500);
}
