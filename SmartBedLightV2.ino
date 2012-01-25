#include "Tlc5940.h"

#define NUM_SAMPLES 4  // how many sample do we need to determine a senser input
#define SAMPLE_THRESHOLD 2 // the threshold of the senser samples
#define SAMPLE_DELAY 1 // the delay time between each sample

#define NUM_LEDS 2 // define the number of LED strips
#define LED_CHANNELS 3 // how many channels does each LED strip have
#define NUM_IRS NUM_LEDS // number of IR sensors, should be the same as LEDs

#define IR_ON HIGH
#define LIGHT_ON LOW
#define LIGHT_OFF HIGH
#define LED_ON HIGH
#define LED_OFF LOW

#define RED 0
#define GREEN 1
#define BLUE 2

// senser pins
#define LIGHT_PIN 7
char IR_PIN[NUM_IRS] = {
  5, 6};

char LED_STRIPS[NUM_LEDS][LED_CHANNELS] = {/*andelle*/
  {
    3,4,5      } 
  , /*compass*/{
    0,1,2      }
};

// LED actions 
#define NONE 0
#define INCREASE 1
#define DECREASE -1

#define TIMEOUT_DELAY 60000 // timeout for turn off led automatically, 1 minus
#define MAX_BRIGHTNESS 4095

char led_status[NUM_LEDS] = {0}; // 0 == off, max == MAX_BRIGHTNESS

char led_action[NUM_LEDS] = {NONE};

unsigned long timer[NUM_LEDS] = {0};

void setup()
{
  /* Call Tlc.init() to setup the tlc.
   You can optionally pass an initial PWM value (0 - 4095) for all channels.*/
  Tlc.init();
  Serial.begin(9600);
  for(int i=0; i< NUM_IRS; i++)
  { 
    pinMode(IR_PIN[i],INPUT);
  }
  pinMode(LIGHT_PIN,INPUT);
}

void loop()
{  
  // read sensors
  int ir_status[NUM_IRS];
  for(int i=0; i<NUM_IRS; i++)
  {
    ir_status[i] = getInput(IR_PIN[i]);
  }
  int light_status = LIGHT_OFF;//getInput(LIGHT_PIN);

  // if light is on, we turn off all leds, unless leds are off
  if(light_status == LIGHT_ON) 
  {
    //Serial.println("Light on");
    for(int i=0; i<NUM_LEDS; i++)
    {
      if(LED_OFF == led_status[i])
      {
        led_action[i] = NONE;
      } 
      else {
        led_action[i] = DECREASE;
      }
    }

    return;
  }

  for(int i=0; i<NUM_LEDS; i++)
  {
     if(IR_ON == ir_status[i]) 
     {
       led_action[i] = INCREASE;
     } else {
       led_action[i] = NONE;
     }
  }
  
  process();
}

// get input according to sample values
boolean getInput(int pin)
{
  int input = 0;
  for(int i=0; i < NUM_SAMPLES; i++)
  {
    input += digitalRead(pin);
    delay(SAMPLE_DELAY);
  }

  return input >> SAMPLE_THRESHOLD;
}

void process()
{
  for(int i=0; i<NUM_LEDS; i++)
  {
    // reach to max brightness, stop increase
    if(MAX_BRIGHTNESS == led_status[i] && INCREASE == led_action[i]) 
    {
      led_action[i] = NONE;
    }

    // if timer times out, we should turn off the led
    if(isTimerOn(i) && isTimeout(i))
    {
      led_action[i] = DECREASE;
    }
  
    if(MAX_BRIGHTNESS == led_status[i] && NONE == led_action[i]) 
    {
      startTimer(i);
    }
    
    led_status[i] += led_action[i];
    
    // decreased to off, reset everything
    if(led_action[i] == DECREASE && LED_OFF == led_status[i])
    {
      led_action[i] = NONE;
      timer[i] = 0;
    }

    Tlc.set(LED_STRIPS[i][RED], led_status[i]);
    Tlc.set(LED_STRIPS[i][GREEN], led_status[i]);
    Tlc.set(LED_STRIPS[i][BLUE], led_status[i]);
    Tlc.update();
    delay(1);
  }
}

void startTimer(int led_index)
{
  timer[led_index] = millis();
}

boolean isTimeout(int led_index)
{
  return timer[led_index] + TIMEOUT_DELAY <= millis();
}


boolean isTimerOn(int led_index)
{
  return timer[led_index] != 0;
}


