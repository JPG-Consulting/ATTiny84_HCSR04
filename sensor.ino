/**
 * ATMEL ATTINY84 / ARDUINO
 *
 *                           +-\/-+
 *                     VCC  1|    |14  GND
 *             (D 10)  PB0  2|    |13  AREF (D  0)
 *             (D  9)  PB1  3|    |12  PA1  (D  1) 
 *       RST           PB3  4|    |11  PA2  (D  2) 
 *  PWM  INT0  (D  8)  PB2  5|    |10  PA3  (D  3) 
 *  PWM        (D  7)  PA7  6|    |9   PA4  (D  4)  SCL
 *  PWM  SDA   (D  6)  PA6  7|    |8   PA5  (D  5)        PWM
 *  
 *  http://electronut.in/talking-to-ultrasonic-distance-sensor-hc-sr04-using-an-attiny84/
 */
#include <TinyWireS.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

// The default buffer size, though we cannot actually affect it by defining it in the sketch
#ifndef TWI_RX_BUFFER_SIZE
#define TWI_RX_BUFFER_SIZE ( 16 )
#endif

#define F_CPU 8000000

#define I2C_SLAVE_ADDRESS 0x4 // the 7-bit address (remember to change this when adapting this example)

// From NewPing
#define MAX_SENSOR_DISTANCE 500 // Maximum sensor distance can be as high as 500cm, no reason to wait for ping longer than sound takes to travel this distance and back. Default=500
#define US_ROUNDTRIP_CM 57      // Microseconds (uS) it takes sound to travel round-trip 1cm (2cm total), uses integer to save compiled code space. Default=57

unsigned int maxEchoTime;
// End NewPing

volatile uint8_t i2c_regs[] =
{
    0x0A, 
    0x01, 
    0x02, 
    0x03, 
};

// Tracks the current register pointer position
volatile byte reg_position;
const byte reg_size = sizeof(i2c_regs);

#define PING_FREQUENCY 33

#define SENSOR_COUNT 3

uint8_t sonar[SENSOR_COUNT][2] = {
  {PA1, PCINT1},
  {PA2, PCINT2},
  {PA3, PCINT3}
};

volatile byte cm[SENSOR_COUNT] = {0, 2, 4};
float distances[SENSOR_COUNT] = {0.0, 0.0, 0.0};
unsigned long distance_timings[SENSOR_COUNT] = {0, 0, 0};
unsigned long nextRead = 0;
uint8_t current_sensor = 0;

volatile unsigned long last_ping_time = 0;
volatile bool echo_done = true;

int32_t ping_emit_time = 0;  // time the ping was emitted in us
volatile int32_t duration;   // duration of the ping

/**
 * This is called for each read request we receive, never put more than one byte of data (with TinyWireS.send) to the 
 * send-buffer when using this callback
 */
void requestEvent()
{  
  // Each read expects X Bytes...
  // The ATTiny85 3 sensor example sends 3 reads, one per sensor...
  // If I want all in one I must loop
  for (byte i=0; i<SENSOR_COUNT; i++) {
    TinyWireS.send(cm[i]);
  }
  
  //TinyWireS.send(i2c_regs[reg_position]);
  
  // Increment the reg position on each read, and loop back to zero
  reg_position++;
  if (reg_position >= reg_size)
  {
    reg_position = 0;
  }
}

float readDistance()
{
  unsigned long maxTime = 0;
  unsigned long startTime = 0;
  unsigned long duration = 0;
  echo_done = false;

  pinMode(sonar[current_sensor][0], OUTPUT);     // Set trigger pin to output.
  digitalWrite(sonar[current_sensor][0], LOW);   // Set the trigger pin low, should already be low, but this will make sure it is.
  delayMicroseconds(4);                          // Wait for pin to go low.
  digitalWrite(sonar[current_sensor][0], HIGH);  // Set trigger pin high, this tells the sensor to send out a ping.
  delayMicroseconds(10);                         // Wait long enough for the sensor to realize the trigger pin is high. Sensor specs say to wait 10uS.
  digitalWrite(sonar[current_sensor][0], LOW);   // Set trigger pin back to low.

  pinMode(sonar[current_sensor][0], INPUT);      // One pin setup
  maxTime = micros() + maxEchoTime;

  // Wait for pulse to start
  while (digitalRead(sonar[current_sensor][0]) == LOW) {
    TinyWireS_stop_check();
    if (micros() > maxTime) {
      current_sensor++;
      if (current_sensor == SENSOR_COUNT) 
      {
        current_sensor = 0;
      }
      return 0;
    }
  }

  startTime = micros();

  // Wait for pulse to end
  while (digitalRead(sonar[current_sensor][0]) == HIGH) {
    TinyWireS_stop_check();
    if (micros() > maxTime) {
      current_sensor++;
      if (current_sensor == SENSOR_COUNT) 
      {
        current_sensor = 0;
      }
      return 0;
    }
  }

  distance_timings[current_sensor] = micros() - startTime;
  cm[current_sensor] = distance_timings[current_sensor] / US_ROUNDTRIP_CM;
  
  current_sensor++;
  if (current_sensor == SENSOR_COUNT) 
  {
    current_sensor = 0;
  }
}

void setup() {
  /**
   * Reminder: taking care of pull-ups is the masters job
   */

  TinyWireS.begin(I2C_SLAVE_ADDRESS);
  //TinyWireS.onReceive(receiveEvent);
  TinyWireS.onRequest(requestEvent);

  // We should take temperature into account!
  maxEchoTime = ((unsigned int)MAX_SENSOR_DISTANCE + 1) * US_ROUNDTRIP_CM; // Calculate the maximum distance in uS (no rounding).

  wdt_enable(WDTO_500MS);               // Watchdog - Reset ATTiny if stuck for more than 500ms
}

void loop() {
  //if (echo_done) {
    readDistance();
  //}
  
  /**
   * This is the only way we can detect stop condition (http://www.avrfreaks.net/index.php?name=PNphpBB2&file=viewtopic&p=984716&sid=82e9dc7299a8243b86cf7969dd41b5b5#984716)
   * it needs to be called in a very tight loop in order not to miss any (REMINDER: Do *not* use delay() anywhere, use tws_delay() instead).
   * It will call the function registered via TinyWireS.onReceive(); if there is data in the buffer on stop.
   */
  TinyWireS_stop_check();

  wdt_reset();                          // feed the watchdog (Tell it we are not stucked)
}
