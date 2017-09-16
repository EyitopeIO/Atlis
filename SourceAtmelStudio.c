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

/* Used to reference the elements in the array */
#define MAX 2
#define NOW 1
#define MIN 0

/************************************************/

void InitializeDataDirection( void );

uint16_t begin_statistical_analysis(int number_of_iterations, int array_size);
void begin_saving_process( int no_of_iterations, int array_size, uint16_t* pointer_to_array, unsigned int address);


void InitializeADC( void );
uint16_t ADCRead( uint8_t ch );

/**********************************************/

const uint8_t ADC_channel = 0;

volatile unsigned char max_button_value; //State of the buttons: high or low.
volatile unsigned char min_button_value;


/* Here are arrays to keep values to save to EEPROM except FromADC0
 * I use 16-bit quantities because the analog values could range from 0-1023
*/ 
volatile uint16_t saved_max[2]; //I defined MAX, MIN, NOW, to make indexing easier.
volatile uint16_t saved_min[2];
volatile uint16_t FromADC0[2]; 

 /*Code banking under consideration*/
unsigned int eeprom_max_address = 0;
unsigned int eeprom_min_address = sizeof(saved_min);

/*************************************************************************/

int main( void ) 
{
	
	InitializeDataDirection();
	InitializeADC();

	/* Fetch the array I saved to the EEPROM. It contains values to run the algorithm*/
	saved_max = eeprom_read_block( (void*)&saved_max, (const void*)eeprom_max_address, sizeof(saved_max) ); 
	saved_min = eeprom_read_block( (void*)&saved_min, (const void*)eeprom_min_address, sizeof(saved_min) );
	
	uint16_t statical_results; //Results from statistical analysis
	
	/* These values below might change--most likely an increase--for better accuracy*/
	uint8_t number_of_iterations = 10; 
	uint8_t number_of_readings = 10; 

	
	while (1) 
    {
		
		// To even out things, whatever number of iterations or readings must be applied to anything that would use it before it's changed.
		
		/*Read the values of the PINx register. It holds the current logic state of the device*/  
		max_button_value = PIND;
		min_button_value = PIND;  
		
		FromADC0[NOW] = begin_statistical_analysis(number_of_iterations, number_of_readings)

		if ( FromADC0[NOW] <= saved_min[NOW] ) {

			statical_results = begin_statistical_analysis(int number_of_iterations, int number_of_readings); //place switch_close in this function
		}
		else if (FromADC0[NOW] >= saved_max[NOW] ) {
			statical_results = begin_statistical_analysis(int number_of_iterations, int number_of_readings) //Place switch_open in this
		}
	}		
}

void begin_saving_process (int no_of_iterations, int array_size, uint16_t* array, unsigned int address) {
	
	/*
	* First parameter is a pointer to the structure. This would either be *p_saved_min or *p_saved_max.
	* Second parameter is pointer to the address you'd like to write to in the EEPROM.
	*/
	
	int start_max = 0;
	int start_min = 1023;
	int tracker = 0;
	uint16_t temp;
	
	saving_on;
	
	do {
		/* Let's say the device took readings when a light was shone on it,
		 * and then the max value goes high. We know this highest isn't authentic.
		 * To correct that, max reduces by 1, min increases by 1
		 */   
		*array[MAX] = *array[MAX] - 1;
		*array[MIN] = *array[MIN] + 1;
		
		temp = ADCRead( ADC_channel ); //Read analog value from ADC.
		if (temp > start_max) {
			*array[MAX] = temp;
		}
		else if (temp < start_min) {
			*array[MIN] = temp;
		}
		tracker++;
		
	} while (tracker < no_of_iterations);
	
	
	/* No harm if the maximum and minimum values are the same */
	*array[NOW] = ( *array[MAX] + *array[MIN] ) / 2;
	 eeprom_write_block ( (const void*) array, (void*) address, sizeof(array) );
	_delay_loop_1(1); //Just so that people can see what's happening. I don't yet know how fast the computation would happen.
	saving_off;	
}


uint16_t begin_statistical_analysis(int number_of_iterations, int array_size) {
	
	/*What this does is get the average of readings for a number of trials*/
	
	int total, array_index, tracker;
	uint16_t Readings[array_size];
	uint16_t average;
	
	/* Initialize all elements to zero */
	for(array_index=0; array_index <= array_size; array_index++) {
		Readings[array_index] = 0;
	}
	
	array_index = 0; //Reset to the first item in the array.
	
	for(tracker=0; tracker < number_of_iterations; tracker++) {
		total = total - Readings[array_index];
		Readings[array_index] = ADCRead(ADC_channel);
		total = total + Readings[array_index];
		if array_index >= array_size {
			array_index = 0;
		}
	average = total / array_size;
	return average;
}

void InitializeDataDirection( void ) {
	
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
}

/* I copied the rest of the code below. */

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

