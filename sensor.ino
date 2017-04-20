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
 *  https://www.arduino.cc/en/Tutorial/MasterReader
 *  http://electronut.in/talking-to-ultrasonic-distance-sensor-hc-sr04-using-an-attiny84/
 */
#if defined (__AVR_ATtiny24__) || defined (__AVR_ATtiny44__) || defined (__AVR_ATtiny84__) || defined (__AVR_ATtiny25__) || defined (__AVR_ATtiny45__) || defined (__AVR_ATtiny85__)
#include <TinyWireS.h>
#else
#include <Wire.h>
#endif
#include <NewPing.h>
#include <avr/wdt.h>         // watchdog

#define I2C_SLAVE_ADDRESS 0x4 // the 7-bit address (remember to change this when adapting this example)

#define SONAR_NUM 3
#define MAX_DISTANCE 200 // Max distance in cm.
#define PING_INTERVAL 33 // Milliseconds between pings.

// Sensor object array.
#if defined (__AVR_ATtiny24__) || defined (__AVR_ATtiny44__) || defined (__AVR_ATtiny84__)
NewPing sonar[SONAR_NUM] = {
  NewPing(PA1, PA1, MAX_DISTANCE),
  NewPing(PA2, PA2, MAX_DISTANCE),
  NewPing(PA3, PA3, MAX_DISTANCE)
};
#elif defined (__AVR_ATtiny25__) || defined (__AVR_ATtiny45__) || defined (__AVR_ATtiny85__)
NewPing sonar[SONAR_NUM] = {
  NewPing(PB1, PB1, MAX_DISTANCE),
  NewPing(PB3, PB3, MAX_DISTANCE),
  NewPing(PB4, PB4, MAX_DISTANCE)
};
#else
NewPing sonar[SONAR_NUM] = {
  NewPing(2, 2, MAX_DISTANCE),
  NewPing(3, 3, MAX_DISTANCE),
  NewPing(4, 4, MAX_DISTANCE)
};
#endif

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
    #if defined (__AVR_ATtiny24__) || defined (__AVR_ATtiny44__) || defined (__AVR_ATtiny84__) || defined (__AVR_ATtiny25__) || defined (__AVR_ATtiny45__) || defined (__AVR_ATtiny85__)  
    TinyWireS.send(cm[i]);
    #else
    Wire.write(cm[i]);
    #endif
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

#if !defined (__AVR_ATtiny24__) && !defined (__AVR_ATtiny44__) && !defined (__AVR_ATtiny84__) && !defined (__AVR_ATtiny25__) && !defined (__AVR_ATtiny45__) && !defined (__AVR_ATtiny85__) 
void debugPrint()
{
  for (byte i=0; i<SONAR_NUM; i++) {
    byte c = cm[i]; 
    if (c < 10) {
      Serial.print("00");
    } else if(c < 100) {
     Serial.print("0");
    }
    Serial.print(c, DEC);         // print the character
    Serial.print(" ");
  }
  Serial.println();
}
#endif

void setup() {
  /**
   * Reminder: taking care of pull-ups is the masters job
   */
  #if defined (__AVR_ATtiny24__) || defined (__AVR_ATtiny44__) || defined (__AVR_ATtiny84__) || defined (__AVR_ATtiny25__) || defined (__AVR_ATtiny45__) || defined (__AVR_ATtiny85__)  
  TinyWireS.begin(I2C_SLAVE_ADDRESS);
  //TinyWireS.onReceive(receiveEvent);
  TinyWireS.onRequest(requestEvent);
  #else
  Serial.begin(9600);
  Wire.begin(I2C_SLAVE_ADDRESS); // join i2c bus
  Wire.onRequest(requestEvent);  // register event
  #endif

  wdt_enable(WDTO_500MS);               // Watchdog - Reset ATTiny if stuck for more than 500ms
}

void loop() {
  if (millis() > nextPing) {
    readDistance();
  }

  #if defined (__AVR_ATtiny24__) || defined (__AVR_ATtiny44__) || defined (__AVR_ATtiny84__) || defined (__AVR_ATtiny25__) || defined (__AVR_ATtiny45__) || defined (__AVR_ATtiny85__)  
  /**
   * This is the only way we can detect stop condition (http://www.avrfreaks.net/index.php?name=PNphpBB2&file=viewtopic&p=984716&sid=82e9dc7299a8243b86cf7969dd41b5b5#984716)
   * it needs to be called in a very tight loop in order not to miss any (REMINDER: Do *not* use delay() anywhere, use tws_delay() instead).
   * It will call the function registered via TinyWireS.onReceive(); if there is data in the buffer on stop.
   */
  TinyWireS_stop_check();
  #else
  // Debug if not a ATTiny
  debugPrint();
  #endif

  wdt_reset();                          // feed the watchdog (Tell it we are not stucked)
}
