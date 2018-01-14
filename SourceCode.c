#include <EEPROM.h>



#define calibrating_off digitalWrite(13, LOW)
#define calibrating_on digitalWrite(13, HIGH)
#define switch_open digitalWrite(8, LOW)
#define switch_close digitalWrite(8, HIGH)
/*#define savedmax_on digitalWrite(10, HIGH)
#define savedmax_off digitalWrite(10, LOW)*/
#define savedmin_on digitalWrite(9, HIGH)
#define savedmin_off digitalWrite(9, LOW)
#define saving_on digitalWrite(7, HIGH)
#define saving_off digitalWrite(7, LOW)

uint8_t channel = A0;
uint16_t fromADC; //from analog read function
unsigned int mini;
//unsigned int maxi;
unsigned int minswitch = 11;
//unsigned int maxswitch = 12;
//unsigned int maxlight = 10;
unsigned int minlight = 9;
unsigned int calibrating_light = 13;
unsigned int saving_light = 7;
unsigned int power = 8;

//uint16_t eeprom_max_address = 0;
uint16_t eeprom_min_address = 20;
uint16_t number_of_iterations = 100; //resolution for getting value saved to eeprom
uint16_t stat;  


void save(unsigned int iterations, unsigned int address);
uint16_t analyse(uint16_t array_size);
uint16_t analyse_mini(uint16_t array_size);

void setup()
{
  pinMode(minlight,OUTPUT); //saved_min light
  ///pinMode(maxlight, OUTPUT); //saved_max light
  pinMode(calibrating_light, OUTPUT); //calibrating light
  pinMode(saving_light, OUTPUT); //saving indicator light
  pinMode(channel , INPUT); //analog input pin
  pinMode(minswitch, INPUT); //min switch
  //pinMode(maxswitch, INPUT); //max switch
  pinMode(power, OUTPUT); //relay
  EEPROM.get(eeprom_min_address, mini);
  //EEPROM.get(eeprom_max_address, maxi);
}

void loop()
{
  calibrating_off;
  //savedmax_on;
  savedmin_on;

  delay(10000);
  /*if (digitalRead(minswitch) == LOW) {
    savedmin_off;
    delay(500);
    save(number_of_iterations, eeprom_min_address);
    savedmin_on;
  }*/
  
  if (digitalRead(minswitch) == LOW) {
    savedmin_off;
    delay(500);
    save(number_of_iterations, eeprom_min_address);
    savedmin_on;
  }
  if (analyse_mini(number_of_iterations) <= mini) {
    switch_close;
    }
  else {
    switch_open;
  }
}
  

void save (unsigned int iter,  unsigned int address)
{
  saving_on;
  stat = analyse(iter);
  EEPROM.put(address, stat);
  delay(500);
  saving_off;
  }


uint16_t analyse(unsigned int array_size)
{
  calibrating_on;
  uint16_t total, array_index;
  uint16_t readings[array_size];
  uint16_t average = 0;
  
  //set all elements to zero
  for(array_index=0; array_index != array_size; array_index++) {
    readings[array_index] = 0;
  }
  
  array_index = 0; //Reset to the first item in the array.
  
  //work out average
  for(total=0; array_index != array_size; array_index++) {
    readings[array_index] = analogRead(channel);
    total = total + readings[array_index];
    array_index++;
    digitalWrite(13,HIGH);
    delay(200);
    digitalWrite(13,LOW);
    delay(200);
  }
  average = total / array_size;
  calibrating_off;
  return average;
}

uint16_t analyse_mini(uint16_t _size)
{
  uint16_t total, array_index;
  uint16_t array_size = _size * 10 ;
  uint16_t readings[array_size];
  uint16_t average = 0;
  
  //set all elements to zero
  for(array_index=0; array_index != array_size; array_index++) {
    readings[array_index] = 0;
  }
  
  array_index = 0; //Reset to the first item in the array.
  
  //work out average
  for(total=0; array_index != array_size; array_index++) {
    readings[array_index] = analogRead(channel);
    total = total + readings[array_index];
    array_index++;
  }
  average = total / array_size;
  return average;
}
