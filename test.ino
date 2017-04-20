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
#include <NewPing.h>
#include <avr/wdt.h>         // watchdog

#define I2C_SLAVE_ADDRESS 0x4 // the 7-bit address (remember to change this when adapting this example)

#define SONAR_NUM 3
#define MAX_DISTANCE 200 // Max distance in cm.
#define PING_INTERVAL 33 // Milliseconds between pings.

// Sensor object array.
NewPing sonar[SONAR_NUM] = {
  NewPing(PA1, PA2, MAX_DISTANCE),
  NewPing(PA3, PA3, MAX_DISTANCE),
  NewPing(PA3, PA3, MAX_DISTANCE)
};

unsigned int cm[SONAR_NUM]; // Store ping distances.
uint8_t currentSensor = 0;  // Which sensor is active.
unsigned long nextPing = 0;

/**
 * This is called for each read request we receive, never put more than one byte of data (with TinyWireS.send) to the 
 * send-buffer when using this callback
 */
void requestEvent()
{  
  // Each read expects X Bytes...
  // The ATTiny85 3 sensor example sends 3 reads, one per sensor...
  // If I want all in one I must loop
  for (byte i=0; i<SONAR_NUM; i++) {
    TinyWireS.send(cm[i]);
  }
}

void readDistance()
{
  unsigned long echoTime = sonar[currentSensor].ping();
  cm[currentSensor] = echoTime / US_ROUNDTRIP_CM;

  nextPing = millis() + PING_INTERVAL;
  
  currentSensor++;
  if (currentSensor == SONAR_NUM) 
  {
    currentSensor = 0;
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
  if (millis() > nextPing) {
    readDistance();
  }
  
  /**
   * This is the only way we can detect stop condition (http://www.avrfreaks.net/index.php?name=PNphpBB2&file=viewtopic&p=984716&sid=82e9dc7299a8243b86cf7969dd41b5b5#984716)
   * it needs to be called in a very tight loop in order not to miss any (REMINDER: Do *not* use delay() anywhere, use tws_delay() instead).
   * It will call the function registered via TinyWireS.onReceive(); if there is data in the buffer on stop.
   */
  TinyWireS_stop_check();

  wdt_reset();                          // feed the watchdog (Tell it we are not stucked)
}
