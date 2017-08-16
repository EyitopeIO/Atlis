/*
 * Generic Atlis.c
 * Author : EyitopeIO_A
 * Created: 17/03/2017 1:33 pm
 * Completed: 21/05/2017 11:45 am
 * Chip: ATmega328P
 */ 

/* Code about the ADC was from http://extremeelectronics.co.in/avr-tutorials/using-adc-of-avr-microcontroller/ before I modified it */

//Consider adding code to set the fuse bits to use an external clock signal

#include <avr/io.h>
#include <util/delay_basic.h>
#define F_CPU 16000000// 8MHz for testing, 16Mhz actual

#define calibrating_off PORTB&=~(1<<PINB5) 
#define calibrating_on PORTB|=(1<<PINB5)
#define switch_open PORTB&=~(1<<PINB0)
#define switch_close PORTB|=(1<<PINB0)
#define savedmax_on PORTB|=(1<<PINB2)
#define savedmax_off PORTB&=~(1<<PINB1)
#define savedmin_on PORTB|=(1<<PINB1)
#define savedmin_off PORTB&=~(1<<PINB1)
#define saving_on PORTB|=(1<<PINB7)
#define saving_off PORTB&=~(1<<PINB7)


void blink (char tag, int on_time, int off_time);
void toggle_switch(void);
void save_new_max(void);
void save_new_min(void);
void calibrate_min(void); 
void calibrate_max(void);
void EEPROM_write(unsigned int uiAddress, unsigned char ucData);
unsigned char EEPROM_read(unsigned int uiAddress);
void InitializeADC(void);
uint16_t ADCRead(uint8_t ch);

uint8_t ADC_channel = 0;
const unsigned int max_address = 4; //Value for max address is saved here.
const unsigned int min_address = 16;
volatile uint8_t max; 
volatile uint8_t min;
volatile unsigned char max_button_value;
volatile unsigned char min_button_value;
volatile uint16_t FromADC0; //analog value read from LDR is a 16 bit quantity

int main(void)
{	
	
	//configure port for input or output
	//The bits are in this order: bit7-bit6-bit5-bit4-bit3-bit2-bit1-bit0
	DDRB = 0b00100111; //0X27 
	DDRC = 0b11111110; //0X00
	DDRD = 0b10011111; //0X9F
	
	//portB0 = status of switch, pin14
	//portB1 = min_is_saved indicator, pin15
	//portB2 = max_is_saved indicator, pin16
	//portB5 = calibrating indicator, pin19
	//portB7 = saving indicator pin13
	//portC0 = analog input from LDR, pin23
	//portD5 = Switch to save min
	//portD6 = Switch to save max
	 
   //Code banking under consideration
	
	InitializeADC();
	max = EEPROM_read(max_address);
	min = EEPROM_read(min_address);
	
	while (1) 
    {
		max_button_value = PORTD6;
		max_button_value = PORTD5; 
		FromADC0 = ADCRead(ADC_channel);
		toggle_switch();
		save_new_max(); //This also checks whether the button is pressed or not
		save_new_min();
		blink('c', 200, 500);	
	}
}

void blink( char PORT, int _on_time, int _off_time) //Use the pin number
{
	/*
	//portB1 = min_is_saved indicator, pin15
	//portB2 = max_is_saved indicator, pin16
	//portB5 = calibrating indicator, pin19
	*/
	unsigned int off_time = _off_time;
	unsigned int on_time = _on_time;
	switch (PORT) {
		case 'n': //min_is_saved_indicator_light
			savedmin_on;
			_delay_loop_2(on_time);
			savedmin_off;
			_delay_loop_2(off_time);
			savedmin_on;
			_delay_loop_2(on_time);
			savedmin_off;
			_delay_loop_2(off_time);
		case 'm': //max_is_saved_indicator_light
			savedmax_on;
			_delay_loop_2(on_time);
			savedmax_off;
			_delay_loop_2(off_time);
			savedmax_on;
			_delay_loop_2(on_time);
			savedmax_off;
			_delay_loop_2(off_time);
		case 'c': //Callibrating_light
			calibrating_on;
			_delay_loop_2(on_time);
			calibrating_off;
			_delay_loop_2(off_time);
			calibrating_on;
			_delay_loop_2(on_time);
			calibrating_off;
			_delay_loop_2(off_time);
	}
}

	
void toggle_switch(void)
{
	if ( ((FromADC0 - min > -10) && (FromADC0 - min < 10)) && (FromADC0 < min) )
	//If (difference between FromSesnor and MIN less than 10) and (brightness less than minimum)
	{
		switch_close;
	}
	if ( ((FromADC0 - max > -10) && (FromADC0 - max < 10)) && (FromADC0 > max) )
	//If (difference between FromADC0 and MAX less than 10) and (brightness greater than maximum)
	{
		switch_open;
	}

}

void calibrate_min(void) 
{
	calibrating_on; //Turn on the calibrating indicator light
	uint8_t count;
	uint8_t temp_min1;
	uint8_t temp_min2;
	uint8_t temp_min3;
	uint8_t temp_min;
	for (count=0; count!=5000 ; count++) {
		temp_min1 = ADCRead(ADC_channel);
		temp_min2 = ADCRead(ADC_channel);
		temp_min3 = ADCRead(ADC_channel);
		temp_min = (temp_min1 + temp_min2 + temp_min3) / 3;
		if (!((temp_min - min > - 10) && (temp_min - min < 10))) //cater for both negative and positive differences
		{ 	
			min = temp_min;
		}
	}
	calibrating_off;
}
void calibrate_max(void)
{
	calibrating_on; //Turn on the calibrating indicator light
	uint8_t count;
	uint8_t temp_max1;
	uint8_t temp_max2;
	uint8_t temp_max3;
	uint8_t temp_max;
	for (count=0; count!=5000 ; count++) {
		temp_max1 = ADCRead(ADC_channel);
		temp_max2 = ADCRead(ADC_channel);
		temp_max3 = ADCRead(ADC_channel);
		temp_max = (temp_max1 + temp_max2 + temp_max3) / 3;
		if (!((temp_max - max > - 10) && (temp_max - max < 10))) 
		{
			max = temp_max;
		}
	}
	calibrating_off;
}

/* I copied the EEPROM read and write codes below from the data sheet*/
void EEPROM_write(unsigned int uiAddress, unsigned char ucData)
{
	/* Wait for completion of previous write */
	while(EECR & (1<<EEPE))
	;
	/* Set up address and Data Registers */
	EEAR = uiAddress;
	EEDR = ucData;
	/* Write logical one to EEMPE */
	EECR |= (1<<EEMPE);
	/* Start eeprom write by setting EEPE */
	EECR |= (1<<EEPE);
}

unsigned char EEPROM_read(unsigned int uiAddress)
{
	/* Wait for completion of previous write */
	while(EECR & (1<<EEPE))
	;
	/* Set up address register */
	EEAR = uiAddress;
	/* Start eeprom read by writing EERE */
	EECR |= (1<<EERE);
	/* Return data from Data Register */
	return EEDR;
}

void InitializeADC()
{
	ADMUX=(1<<REFS0);                         // For Aref=AVcc;
	ADCSRA=(1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0); //Prescalar div factor =128
}

uint16_t ADCRead (uint8_t ch) //Reading from Channel zero
{
	//Select ADC Channel ch must be 0-7
	ch=ch&0b00000111;
	ADMUX|=ch;

	//Start Single conversion
	ADCSRA|=(1<<ADSC);

	//Wait for conversion to complete
	while(!(ADCSRA & (1<<ADIF)));

	//Clear ADIF by writing one to it
	//Note you may be wondering why we have write one to clear it
	//This is standard way of clearing bits in io as said in datasheets.
	//The code writes '1' but it result in setting bit to '0' !!!

	ADCSRA|=(1<<ADIF);

	return(ADC);
} 


void save_new_max(void) //max_is_saved_indicator_light = B2
{
	if (max_button_value == 0)
	{
		savedmax_off;
		_delay_loop_2(500); //Just so that users can see what's happening
		saving_on;
		calibrate_max();
		EEPROM_write(max_address, max);
		max = EEPROM_read(max_address); //Reset the global value
		saving_off;
		blink('m', 200, 500);
		savedmax_on;
		
	}
}

void save_new_min(void) //min_is_saved_indicator_light = B1
{
	if (min_button_value == 0)
	{
		savedmin_off;
		_delay_loop_2(500); //Just so that users can see what's happening
		saving_on;
		calibrate_min();
		EEPROM_write(min_address, min);
		min = EEPROM_read(min_address); //Reset the global value
		saving_off;
		blink('n', 200,500);
		savedmin_on;
		
	}
}
