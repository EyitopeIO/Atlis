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
#include <avr/eeprom.h>

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
#define MAX 2
#define NOW 1
#define MIN 0


/************************************************/

uint16_t begin_statistical_analysis(int number_of_iterations, int number_of_readings);
void begin_saving_process(int no_of_iterations, int no_of_readings);


void InitializeADC(void);
uint16_t ADCRead(uint8_t ch);

/**********************************************/

//unsigned char EEMEM FIRST_RUN = 0x00; To know if it's the first time this program would ever run.

const uint8_t ADC_channel = 0;

volatile unsigned char max_button_value; //State of the buttons: high or low.
volatile unsigned char min_button_value;

volatile uint16_t FromADC0; //analog value read from LDR is a 16 bit quantity

/* Here are arrays to keep values to save to EEPROM
 * I use 16-bit quantities because numbers could range from 0-1023
*/ 
uint16_t saved_max[2];  
uint16_t saved_min[2];

int eeprom_max_address = 0;
int eeprom_min_address = sizeof(saved_min);

/*************************************************************************/

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
	saved_max = eeprom_read_block(); 
	saved_min = eeprom_read_block();
	
	uint16_t statical_results; //Results from statistical analysis
	
	int number_of_iterations = 10; //This might increase or decrease (by pointers) based on whether readings are reliable or not.
	int number_of_readings = 10; //This might also change as above.
	
	int *p_n_o_iter = &number_of_iterations;
	int *p_n_o_read = &number_of_readings;

	unsigned int mid_of_max_and_min = (max + min) / 2; 
	unsigned char is_day = TRUE; //Works as a boolean to let me know if it's day. We should assume it's day
	
	while (1) 
    {
		
		// To even out things, whatever number of iterations or readings must be applied to all cases before being changed.
		
		max_button_value = PORTD6;
		min_button_value = PORTD5; 
		/*Use an interrupt to deal with the button press.
		*The interrupt should trigger a variable that would be used to save or not save.
		*/
		FromADC0.now = begin_statistical_analysis(number_of_iterations, number_of_readings)

		if ( FromADC0 <= saved_min.now ) {
			statical_results = begin_statistical_analysis(int number_of_iterations, int number_of_readings); //place switch_close in this function
		}
		else if (FromADC0 >= saved_max.now ) {
			statical_results = begin_statistical_analysis(int number_of_iterations, int number_of_readings) //Place switch_open in this
		}
	}		
}

void begin_saving_process (int no_of_iterations, int array_size) {
	
	/*
	* First parameter is a pointer to the structure. This would either be *p_saved_min or *p_saved_max.
	* Second parameter is pointer to the address you'd like to write to in the EEPROM.
	*/
	
	int start_max = 0;
	int start_min = 1023;
	char tracker;
	uint16_t temp;
	
	saving_on;
	
	for (tracker=0; tracker <= no_of_iterations; tracker++ ) {
		temp = begin_statistical_analysis(no_of_iterations, array_size);
	}
	
	saving_off;	
}


uint16_t begin_statistical_analysis(int number_of_readings, int array_size) {
	
	/*What this does is get the average of readings for a number of trials*/
	
	int total, array_index, tracker;
	uint16_t Readings[array_size];
	uint16_t average;
	
	for(array_index=0; array_index <= array_size; array_index++) {
		Readings[array_index] = 0;
	}
	array_index = 0;
	for(tracker=0; tracker < number_of_readings; tracker++) {
		total = total - Readings[array_index];
		Readings[array_index] = ADCRead(ADC_channel);
		total = total + Readings[array_index];
		if array_index >= array_size {
			array_index = 0;
		}
	average = total / array_size;
	return average;
}



void InitializeADC()
{
	ADMUX=(1<<REFS0);                         // For Aref=AVcc;
	ADCSRA=(1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0); //Prescalar div factor =128
}

uint16_t ADCRead (uint8_t ch) //Reading from Channel zero
{
	//Select ADC Channel ch must be 0-7
	ch = ch & 0b00000111;
	ADMUX |= ch;

	//Start Single conversion
	ADCSRA |= (1<<ADSC);

	//Wait for conversion to complete
	while(!(ADCSRA & (1<<ADIF)));

	//Clear ADIF by writing one to it
	//Note you may be wondering why we have write one to clear it
	//This is standard way of clearing bits in io as said in datasheets.
	//The code writes '1' but it result in setting bit to '0' !!!

	ADCSRA|=(1<<ADIF);

	return(ADC);
} 

