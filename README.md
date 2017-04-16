# ATTiny84_HCSR04

NOT WORKING

This is just a backup of the work in progress to build an I2C sensor using ATTiny84.

The sensor will be composed of multiple HC-SR04 and a temperature sensor.

The temperature sensor will be used to compensate the sound speed as it changes due to temperature.

The HC-SR04 sensors will be used to measure the distances at diferent angles.

The master should be able to read the distances as fast as possible to avoid obstacles.

I would also want to add a IRQ (Interrupt Request) pin which will be triggered if a distance is under a certain distances. Kind of an emergency stop pin.

`sensor.old` - The interrupts didn't work as expected...
`sensor.ino` - I get readings but they are WAY OUT of accurracy.
