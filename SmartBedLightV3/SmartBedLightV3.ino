// You can choose the latch pin yourself.
const int ShiftPWM_latchPin=10;

// ** uncomment this part to NOT use the SPI port and change the pin numbers. This is 2.5x slower **
// #define SHIFTPWM_NOSPI
// const int ShiftPWM_dataPin = 11;
// const int ShiftPWM_clockPin = 13;


// If your LED's turn on if the pin is low, set this to true, otherwise set it to false.
const bool ShiftPWM_invertOutputs = false;

// You can enable the option below to shift the PWM phase of each shift register by 8 compared to the previous.
// This will slightly increase the interrupt load, but will prevent all PWM signals from becoming high at the same time.
// This will be a bit easier on your power supply, because the current peaks are distributed.
const bool ShiftPWM_balanceLoad = false;

#include <ShiftPWM.h>   // include ShiftPWM.h after setting the pins!

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

#define BRIGHTNESS_STEP 1

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

#define TIMEOUT_DELAY 5000 // timeout for turn off led automatically, 1 minus
#define MAX_BRIGHTNESS 255

int led_status[NUM_LEDS] = {0}; // 0 == off, max == MAX_BRIGHTNESS

//char led_action[NUM_LEDS] = {NONE};

unsigned long timer[NUM_LEDS] = {0};

// ShiftPWM related variables
unsigned char pwmFrequency = 75;
unsigned int numRegisters = 1;
unsigned int numOutputs = numRegisters*8;
unsigned int numRGBLeds = numRegisters*8/3;
unsigned int fadingMode = 0; //start with all LED's off.

void setup()
{
  //Serial.begin(9600);  

  for(int i=0; i< NUM_IRS; i++)
  { 
    pinMode(IR_PIN[i],INPUT);
  }
  pinMode(LIGHT_PIN,INPUT);
  
  // Sets the number of 8-bit registers that are used.
  ShiftPWM.SetAmountOfRegisters(numRegisters);

  // SetPinGrouping allows flexibility in LED setup. 
  // If your LED's are connected like this: RRRRGGGGBBBBRRRRGGGGBBBB, use SetPinGrouping(4).
  ShiftPWM.SetPinGrouping(1); //This is the default, but I added here to demonstrate how to use the funtion
  
  ShiftPWM.Start(pwmFrequency,MAX_BRIGHTNESS);
  ShiftPWM.SetAll(0);
  
  // self testing
  delay(1000);
  ShiftPWM.OneByOneSlow();
  ShiftPWM.SetAll(MAX_BRIGHTNESS);
  delay(200);
  ShiftPWM.SetAll(0);
  delay(200);
  ShiftPWM.SetAll(MAX_BRIGHTNESS);
  delay(200);
  ShiftPWM.SetAll(0);
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
      if(LED_OFF != led_status[i])
      {
        turnOffLed(i);
      }
    }

    return;
  }

  for(int i=0; i<NUM_LEDS; i++)
  {
     //if(ir_status[i] == IR_ON) {Serial.print("IR status for "); Serial.print(i); Serial.print(" is "); Serial.println(ir_status[i]);}
     if(IR_ON == ir_status[i]) 
     {
       turnOnLed(i);
       startTimer(i);
     } else {
       if(isTimerOn(i) && isTimeout(i)) 
       {
         turnOffLed(i);
         stopTimer(i);
       }
     }
  }
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

void startTimer(int led_index)
{
  //Serial.print("Timer started on "); Serial.println(led_index);
  timer[led_index] = millis();
}

void stopTimer(int led_index)
{
  timer[led_index] = 0;
}

boolean isTimeout(int led_index)
{
  //Serial.print("Timer off on "); Serial.println(led_index);
  return timer[led_index] + TIMEOUT_DELAY <= millis();
}

boolean isTimerOn(int led_index)
{
  return timer[led_index] != 0;
}

void turnOnLed(int led_index)
{
  for(int brightness = led_status[led_index]; brightness<=MAX_BRIGHTNESS; brightness+=BRIGHTNESS_STEP) 
  {
    ShiftPWM.SetOne(LED_STRIPS[led_index][RED], brightness);
    ShiftPWM.SetOne(LED_STRIPS[led_index][GREEN], brightness);
    ShiftPWM.SetOne(LED_STRIPS[led_index][BLUE], brightness);
    delay(1);
  }
  led_status[led_index] = MAX_BRIGHTNESS;
}

void turnOffLed(int led_index)
{
  for(int brightness = led_status[led_index]; brightness>=0; brightness-=BRIGHTNESS_STEP) 
  {
    ShiftPWM.SetOne(LED_STRIPS[led_index][RED], brightness);
    ShiftPWM.SetOne(LED_STRIPS[led_index][GREEN], brightness);
    ShiftPWM.SetOne(LED_STRIPS[led_index][BLUE], brightness);
    delay(1);
  }
  led_status[led_index] = 0;
}
