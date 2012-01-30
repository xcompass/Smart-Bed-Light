/*
    Basic Pin setup:
    ------------                                  ---u----
    ARDUINO   13|-> SCLK (pin 25)           OUT1 |1     28| OUT channel 0
              12|                           OUT2 |2     27|-> GND (VPRG)
              11|-> SIN (pin 26)            OUT3 |3     26|-> SIN (pin 11)
              10|-> BLANK (pin 23)          OUT4 |4     25|-> SCLK (pin 13)
               9|-> XLAT (pin 24)             .  |5     24|-> XLAT (pin 9)
               8|                             .  |6     23|-> BLANK (pin 10)
               7|                             .  |7     22|-> GND
               6|                             .  |8     21|-> VCC (+5V)
               5|                             .  |9     20|-> 2K Resistor -> GND
               4|                             .  |10    19|-> +5V (DCPRG)
               3|-> GSCLK (pin 18)            .  |11    18|-> GSCLK (pin 3)
               2|                             .  |12    17|-> SOUT
               1|                             .  |13    16|-> XERR
               0|                           OUT14|14    15| OUT channel 15
    ------------                                  --------

    -  Put the longer leg (anode) of the LEDs in the +5V and the shorter leg
         (cathode) in OUT(0-15).
    -  +5V from Arduino -> TLC pin 21 and 19     (VCC and DCPRG)
    -  GND from Arduino -> TLC pin 22 and 27     (GND and VPRG)
    -  digital 3        -> TLC pin 18            (GSCLK)
    -  digital 9        -> TLC pin 24            (XLAT)
    -  digital 10       -> TLC pin 23            (BLANK)
    -  digital 11       -> TLC pin 26            (SIN)
    -  digital 13       -> TLC pin 25            (SCLK)
    -  The 2K resistor between TLC pin 20 and GND will let ~20mA through each
       LED.  To be precise, it's I = 39.06 / R (in ohms).  This doesn't depend
       on the LED driving voltage.
    - (Optional): put a pull-up resistor (~10k) between +5V and BLANK so that
                  all the LEDs will turn off when the Arduino is reset.

    If you are daisy-chaining more than one TLC, connect the SOUT of the first
    TLC to the SIN of the next.  All the other pins should just be connected
    together:
        BLANK on Arduino -> BLANK of TLC1 -> BLANK of TLC2 -> ...
        XLAT on Arduino  -> XLAT of TLC1  -> XLAT of TLC2  -> ...
    The one exception is that each TLC needs it's own resistor between pin 20
    and GND.

    This library uses the PWM output ability of digital pins 3, 9, 10, and 11.
    Do not use analogWrite(...) on these pins.

    This sketch does the Knight Rider strobe across a line of LEDs.

    Alex Leone <acleone ~AT~ gmail.com>, 2009-02-03 */

#include "Tlc5940.h"

#define IR_ON HIGH
#define LIGHT_ON LOW
#define LIGHT_OFF HIGH
#define LED_ON HIGH
#define LED_OFF LOW

// senser pins
#define IR_ANDELLE_PIN 5
#define IR_COMPASS_PIN 6
#define LIGHT_PIN 7

// LED Pins for compass
#define LED_COMPASS_RED 0
#define LED_COMPASS_GREEN 1
#define LED_COMPASS_BLUE 2

// LED Pins for andelle
#define LED_ANDELLE_RED 3
#define LED_ANDELLE_GREEN 4
#define LED_ANDELLE_BLUE 5

#define ONE_MINUTE 60000

int last_status_compass = 0;
int last_status_andelle = 0;

void setup()
{
  /* Call Tlc.init() to setup the tlc.
     You can optionally pass an initial PWM value (0 - 4095) for all channels.*/
  Tlc.init();
  Serial.begin(9600);
  pinMode(IR_COMPASS_PIN,INPUT);
  pinMode(IR_ANDELLE_PIN,INPUT);
  pinMode(LIGHT_PIN,INPUT);
}

/* This loop will create a Knight Rider-like effect if you have LEDs plugged
   into all the TLC outputs.  NUM_TLCS is defined in "tlc_config.h" in the
   library folder.  After editing tlc_config.h for your setup, delete the
   Tlc5940.o file to save the changes. */

void loop()
{    
  int ir_compass_status = digitalRead(IR_COMPASS_PIN);
  int ir_andelle_status = digitalRead(IR_ANDELLE_PIN);
  int light_status = LIGHT_OFF;//digitalRead(LIGHT_PIN);
  
  // debug  
  //Serial.print(light_status); Serial.print("\t");Serial.print(ir_compass_status);Serial.print("\t");Serial.print(ir_andelle_status);Serial.print("\t");Serial.print(last_status_compass);Serial.print("\t");Serial.println(last_status_andelle);
    
  if(light_status == LIGHT_ON) 
  {
    //Serial.println("Light on");
    if (last_status_compass == LED_ON && last_status_andelle == LED_ON) 
    {
      turnOffAllLed();
    } else if (last_status_compass == LED_ON)
    {
      turnOffCompassLed();
    } else if (last_status_andelle == LED_ON)
    {
      turnOffAndelleLed();
    }
    return;
  }
  
  // light off
  if(ir_compass_status == IR_ON) {
    // detected movement, turn on led
    turnOnCompassLed();
    delay(1000);
  } else {
    // turn off the led
    turnOffCompassLed();
  }
  
    // light off
  if(ir_andelle_status == IR_ON) {
    // detected movement, turn on led
    turnOnAndelleLed();
    delay(1000);
  } else {
    // turn off the led
    turnOffAndelleLed();
  }
}

void turnOnCompassLed() {
  if (last_status_compass == LED_ON) return;
  for(int i = 0; i < 4095; i++)
  {
    Tlc.set(LED_COMPASS_RED, i);
    Tlc.set(LED_COMPASS_GREEN, i);
    Tlc.set(LED_COMPASS_BLUE, i);
    Tlc.update();
    delay(1);
  }
  delay(ONE_MINUTE);
  last_status_compass = LED_ON;
}

void turnOnAndelleLed() {
  if (last_status_andelle == LED_ON) return;
  for(int i = 0; i < 4095; i++)
  {
    Tlc.set(LED_ANDELLE_RED, i);
    Tlc.set(LED_ANDELLE_GREEN, i);
    Tlc.set(LED_ANDELLE_BLUE, i);
    Tlc.update();
    delay(1);
  }
  delay(ONE_MINUTE);
  last_status_andelle = LED_ON;
}

void turnOnAllLed() {
  if (last_status_compass == LED_ON) return;
  for(int i = 0; i < 4095; i++)
  {
    Tlc.set(LED_COMPASS_RED, i);
    Tlc.set(LED_COMPASS_GREEN, i);
    Tlc.set(LED_COMPASS_BLUE, i);
    Tlc.set(LED_ANDELLE_RED, i);
    Tlc.set(LED_ANDELLE_GREEN, i);
    Tlc.set(LED_ANDELLE_BLUE, i);
    Tlc.update();
    delay(1);
  }
  delay(ONE_MINUTE);
  last_status_compass = LED_ON;
}

void turnOffCompassLed() {
  if (last_status_compass == LED_OFF) return;
  //Serial.println("Turning off LED");
  for(int i = 4095; i >= 0; i--)
  {
    Tlc.set(LED_COMPASS_RED, i);
    Tlc.set(LED_COMPASS_GREEN, i);
    Tlc.set(LED_COMPASS_BLUE, i);
    Tlc.update();
    delay(1);
  }
  last_status_compass = LED_OFF;
}

void turnOffAndelleLed() {
  if (last_status_andelle == LED_OFF) return;
  //Serial.println("Turning off LED");
  for(int i = 4095; i >= 0; i--)
  {
    Tlc.set(LED_ANDELLE_RED, i);
    Tlc.set(LED_ANDELLE_GREEN, i);
    Tlc.set(LED_ANDELLE_BLUE, i);
    Tlc.update();
    delay(1);
  }
  last_status_andelle = LED_OFF;
}

void turnOffAllLed() {
  if (last_status_compass == LED_OFF && last_status_andelle == LED_OFF) return;
  //Serial.println("Turning off LED");
  for(int i = 4095; i >= 0; i--)
  {
    if(last_status_compass == LED_ON) {
      Tlc.set(LED_COMPASS_RED, i);
      Tlc.set(LED_COMPASS_GREEN, i);
      Tlc.set(LED_COMPASS_BLUE, i);
    }
    
    if(last_status_andelle == LED_ON) {
      Tlc.set(LED_ANDELLE_RED, i);
      Tlc.set(LED_ANDELLE_GREEN, i);
      Tlc.set(LED_ANDELLE_BLUE, i);
    }
    Tlc.update();
    delay(1);
  }
  last_status_compass = LED_OFF;
  last_status_andelle = LED_OFF;
}
