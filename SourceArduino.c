/*
*Author: EyitopeIO_A
*Date & Time: May 21 2017, 1:07 pm
*/

#include <Arduino.h>
#include <EEPROM.h>


#define switch_open digitalWrite(IsDeliveringPower,LOW)
#define switch_close digitalWrite(IsDeliveringPower,HIGH)
#define calibrating_on digitalWrite(calibrating_light,HIGH)
#define calibrating_off digitalWrite(calibrating_light,LOW)
#define savedmax_off digitalWrite(max_is_saved_indicator_light, LOW)
#define savedmax_on digitalWrite(max_is_saved_indicator_light, HIGH)
#define savedmin_on digitalWrite(min_is_saved_indicator_light, HIGH)
#define savedmin_off digitalWrite(min_is_saved_indicator_light, LOW)
#define saving_on digitalWrite(saving, HIGH)
#define saving_off digitalWrite(saving, LOW)

//Don't forget to change it from being a constant if you're implementing code banking
const int min_address = 22; //This address would be changed by the code banking function. It's under consideration
const int max_address = 10; //Same comment as above
volatile int max_button_value; //The state of the button as read from digitalRead(max_button)
volatile int min_button_value; //Similar to what's above
const int max_button = 6;
const int min_button = 5;
const int calibrating_light = 13;
const int saving = 7;
const int IsDeliveringPower = 8;
const int max_is_saved_indicator_light = 10;
const int min_is_saved_indicator_light = 9;
const int sensor = A0;
volatile int FromSensor; //value from analogRead()
volatile int MAX; 
volatile int MIN;

void toggle_switch(void);
void save_new_max(void);
void save_new_min(void);
void calibrate_min(void);
void calibrate_max(void);
void blink(int pin_number, int off_time, int on_time);

void setup()
{
  pinMode(calibrating_light, OUTPUT);
  pinMode(IsDeliveringPower, OUTPUT);
  pinMode(saving, OUTPUT);
  pinMode(sensor, INPUT);
  pinMode(max_button, INPUT);
  pinMode(min_button, INPUT);
  pinMode(max_is_saved_indicator_light, OUTPUT);
  pinMode(min_is_saved_indicator_light, OUTPUT);
 

   /* All new devices would come with a saved value already. Code banking under consideration here
  *
  * I previously put the statement to read value from EEPROM here. This had the effect of making the new value useless until a reset was initiated.
  */
  EEPROM.get(max_address, MAX);
  EEPROM.get(min_address, MIN);
  savedmin_on;
  savedmax_on;
}


void loop()
{  
  min_button_value = digitalRead(min_button);
  max_button_value  = digitalRead(max_button);
  FromSensor = analogRead(sensor);
  toggle_switch();
  save_new_max(); //This also checks whether the button is pressed or not
  save_new_min();
  blink(calibrating_light, 500, 500); 
}

void blink( int pin_number, int on_time, int off_time) //Use the pin number
{
	/*
	const int calibrating_light = 13;
	const int saving = 7;
	const int IsDeliveringPower = 8;
	const int max_is_saved_indicator_light = 10;
	const int min_is_saved_indicator_light = 9;
	*/
	unsigned int light = pin_number;
	unsigned int _on_time = on_time;
	unsigned int _off_time = off_time;
	digitalWrite(light, HIGH);
	delay(_on_time);
	digitalWrite(light, LOW);
	delay(_off_time);
	digitalWrite(light, HIGH);
	delay(_on_time);
	digitalWrite(light, LOW);
	delay(_off_time);
}

void toggle_switch(void)
{
  if ( ((FromSensor - MIN < -10) || (FromSensor - MIN > 10)) && (FromSensor < MIN) )
  //If (difference between FromSesnor and MIN is greater than 10) and (brightness less than minimum)
  {
    switch_close;
  }
  if ( ((FromSensor - MAX < -10) || (FromSensor - MAX > 10)) && (FromSensor > MAX) )
  //If (difference between FromSensor and MAX greater than 10) and (brightness greater than maximum)
  {
    switch_open;
  }

}


void calibrate_min(void)
{
  calibrating_on; //Turn on the calib*9rating indicator light
  int count;
  int temp_min1;
  int temp_min2;
  int temp_min3;
  int temp_min;
  for (count=0; count!=10000 ; count++) {
    temp_min1 = analogRead(sensor);
    temp_min2 = analogRead(sensor);
    temp_min3 = analogRead(sensor);
    temp_min = (temp_min1 + temp_min2 + temp_min3) / 3;
    if (!((temp_min - MIN > - 10) && (temp_min - MIN < 10))) //cater for both negative and positive differences
    {
      MIN = temp_min;
    }
  }
  calibrating_off;
}


void calibrate_max(void)
{
  calibrating_on; //Turn on the calibrating indicator light
  int count;
  int temp_max1;
  int temp_max2;
  int temp_max3;
  int temp_max;
  for (count=0; count!=10000 ; count++) {
    temp_max1 = analogRead(sensor);
    temp_max2 = analogRead(sensor);
    temp_max3 = analogRead(sensor);
    temp_max = (temp_max1 + temp_max2 + temp_max3) / 3;
    if (!((temp_max - MAX > - 10) && (temp_max - MAX < 10)))
    {
      MAX = temp_max;
    }
  }
  calibrating_off;
}


void save_new_max(void)
{
  if (max_button_value == LOW)
  {
    savedmax_off;
    delay(500); //Just so that users can see what's happening
    saving_on;
    calibrate_max();
    EEPROM.put(max_address, MAX);
	EEPROM.get(max_address, MAX); //Reset the global value 
	blink(max_is_saved_indicator_light, 200, 200);
    saving_off;
	delay(500);
	savedmax_on;
	
  }
}


void save_new_min(void)
{
  if (min_button_value == LOW)
  {
    savedmin_off;
    delay(500); //Just so that users can see what's happening
    saving_on;
    calibrate_min();
    EEPROM.put(min_address, MIN);
	EEPROM.get(min_address, MIN); //Reset the global value
	blink(min_is_saved_indicator_light, 200, 200);
    saving_off;
	delay(500);
	savedmin_on;
  }
}
