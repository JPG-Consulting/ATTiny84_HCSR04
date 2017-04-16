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

float distances[SENSOR_COUNT] = {0.0, 0.0, 0.0};
uint8_t current_sensor = 0;

volatile unsigned long last_ping_time = 0;
volatile bool echo_done = false;

int32_t ping_emit_time = 0;  // time the ping was emitted in us
volatile int32_t duration;   // duration of the ping

/**
 * the ping pin will flip HIGH at the point when the pulse has completed
 * and timing should begin, it will then flip LOW once the sound wave is received
 * so we need to detect both of these states
 */
ISR(PCINT0_vect) 
{
  if (PINA & (1<<sonar[current_sensor][0])) {
    // rising edge:
    ping_emit_time = micros();
  } else { 
    // falling edge:
    duration = micros() - ping_emit_time;

    // set flag
    echo_done = true;
  }
}

/**
 * This is called for each read request we receive, never put more than one byte of data (with TinyWireS.send) to the 
 * send-buffer when using this callback
 */
void requestEvent()
{  
  // Each read expects X Bytes...
  // The ATTiny85 3 sensor example sends 3 reads, one per sensor...
  // If I want all in one I must loop
  for (byte i=0; i<reg_size; i++) {
    TinyWireS.send(i2c_regs[i]);
  }
  //TinyWireS.send(i2c_regs[reg_position]);
  //TinyWireS.send(reg_position);
  
  // Increment the reg position on each read, and loop back to zero
  reg_position++;
  if (reg_position >= reg_size)
  {
    reg_position = 0;
  }
}

float readDistance()
{
  float distance = 0.0f;
    
  echo_done = false;  // set echo flag
  //count_timer0 = 0;   // reset counter
    
  // Trigger
  DDRA |= (1 << sonar[current_sensor][0]); //set PA7 as output
  PORTA &= ~(1 << sonar[current_sensor][0]);//set PA7 low
  _delay_us(20);
  PORTA |= (1 << sonar[current_sensor][0]); //set pin PA7 high
  _delay_us(12);
  PORTA &= ~(1 << sonar[current_sensor][0]);//set PA7 low
  _delay_us(20);

  // Set PA7 as input
  DDRA &= ~(1 << sonar[current_sensor][0]); // Set PA7 as input

  // we'll use a pin change interrupt to notify when the
  // ping pin changes rather than being blocked by
  // Arduino's built-in pulseIn function
  cli();                 // disable all interrupts temporarily
  GIMSK  |= _BV(PCIE0);  // Enable Pin Change Interrupts
  PCMSK0 |= _BV(sonar[current_sensor][1]); // Use PA7 as interrupt pin
  sei();                 // re-enable all interrupts

  while (!echo_done) 
  {
    TinyWireS_stop_check();
  }
      
  // disable interrupt while pinMode is OUTPUT
  // not sure if this is actually necessary, but just
  // playing it safe to avoid false interrupt when
  // pin mode is OUTPUT
  cli();                   // disable all interrupts temporarily
  GIMSK  &= ~_BV(PCIE0);   // Disable Pin Change Interrupts
  PCMSK0 &= ~_BV(sonar[current_sensor][1]);  // Disable pin change interrupt on PA7
  sei();
   
  // dist = duration * speed of sound * 1/2
  // dist in cm = duration in s * 340.26 * 100 * 1/2
  // = 17013*duration
  distance = 17013.0 * duration;

  if (distance <= 500) 
  {
    distances[current_sensor] = distance;
  }
  
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

  wdt_enable(WDTO_500MS);               // Watchdog - Reset ATTiny if stuck for more than 500ms
}

void loop() {
  //readDistance();
  /**
   * This is the only way we can detect stop condition (http://www.avrfreaks.net/index.php?name=PNphpBB2&file=viewtopic&p=984716&sid=82e9dc7299a8243b86cf7969dd41b5b5#984716)
   * it needs to be called in a very tight loop in order not to miss any (REMINDER: Do *not* use delay() anywhere, use tws_delay() instead).
   * It will call the function registered via TinyWireS.onReceive(); if there is data in the buffer on stop.
   */
  TinyWireS_stop_check();

  wdt_reset();                          // feed the watchdog (Tell it we are not stucked)
}
