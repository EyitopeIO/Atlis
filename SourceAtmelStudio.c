/*
 * Generic Atlis.c
 * Author : EyitopeIO_A
 * Created: 17/03/2017 1:33 pm
 * 
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

uint16_t begin_statistical_analysis( int number_of_iterations, int array_size );
void begin_saving_process( int no_of_iterations, int array_size, uint16_t* pointer_to_array, unsigned int address );

void Initialize_ADC( void );
uint16_t Read_from_ADC( uint8_t channel );

int EEPROM_handler( char mode, uint16_t address, uint16_t* array); 

/**********************************************/

volatile unsigned char max_button_value; //State of the buttons: high or low.
volatile unsigned char min_button_value;

/* Here are arrays to keep values to save to EEPROM except FromADC0
 * I use 16-bit quantities because the analog values could range from 0-1023
 */ 
uint16_t saved_max[MAX]; //I defined MAX, MIN, NOW, to make indexing obvious.
uint16_t saved_min[MAX];
uint16_t FromADC0; 

uint8_t ch = 0x00; //Just felt like using a hex.


/*************************************************************************/

int main( void ) 
{
	
	InitializeDataDirection();
	Initialize_ADC();
	
	/*Code banking under consideration*/
	uint16_t eeprom_max_address = 0; //it is 16 bit because of the function that would use it.
	uint16_t eeprom_min_address = sizeof(saved_max);
	
	/* These values below might change--most likely an increase--for better accuracy*/
	uint8_t number_of_iterations = 10;
	uint8_t number_of_readings = 10;
	
	uint16_t statistical_results;

	/* Fetch the array I saved to the EEPROM. It contains values to run the algorithm*/
	EEPROM_handler( 'r', eeprom_max_address, saved_max); //int EEPROM_handler( char mode, uint16_t address, uint16_t* array )
	EEPROM_handler( 'r', eeprom_min_address, saved_min); 
	
	
	while (1) 
    {
		// To even out things, whatever number of iterations or readings must be applied to anything that would use it before it's changed.
		
		 /* This is the logic: 
		  * 1) Read from ADC.
		  * 2) Compare read value with saved maximum. If it's greater, do a more accurate reading.
		  *    For saved minimum, do a more accurate reading if it's lower.
		  * 3) If it's neither greater than max or less than min, number_of_iterations and number_of_readings.
		  *
		  */
		max_button_value = PIND6; //PIND6
		min_button_value = PIND5; //PIND5
		
		/*Code to check state of buttons here*/
		if (PIND5 == 0) {
			begin_saving_process(number_of_iterations number_of_readings,saved_min, eeprom_min_address);
		}
		else if (PIND6 == 0) {
			begin_saving_process(number_of_iterations, number_of_readings, saved_max, eeprom_max_address);
		}
		.   
		FromADC0 = Read_from_ADC( ch );
		
		if (FromADC0 > saved_max[NOW]) {
			//Add code to make the calibrating light blink very fast using timers to show that the device is busy.
			statistical_results = begin_statistical_analysis( number_of_iterations, number_of_readings );
			if (statistical_results >= saved_max[NOW]) {
				while( !(FromADC0 <= saved_min[NOW]) ) { //Don't exit for as long as its bright
					FromADC0 = Read_from_ADC( ch );
					switch_open;
				}
			}
			else {
				number_of_iterations++; //Increase so that you'd be more precise next time because you were deceived.
				number_of_readings++;
			}
		}
		
		else if (FromADC0 < saved_min[NOW]) {
			//Add code to make the calibrating light blink very fast using timers. It shows device is busy.
			statistical_results = begin_statistical_analysis( number_of_iterations, number_of_readings );
			if (statistical_results <= saved_min[NOW]) {
				while( !(FromADC0 >= saved_max[NOW]) ) {
					FromADC0 = Read_from_ADC ( ch );
					switch_close;
				}
			}
			else {
				number_of_iterations++;
				number_of_readings++;
			}
		}
		
		else {
			number_of_iterations = 10;
			number_of_readings = 10;
		}
	}
}
					


int EEPROM_handler( char mode, uint16_t address, uint16_t* array ) {
	
	int tracker;
	switch(mode) {
		
		case 'r': //read
		for (tracker=0; tracker != MAX; tracker++) {
			*(array+tracker) = eeprom_read_word ( (const uint16_t*) address );
			address = address + sizeof( *(array+tracker) );
		}
		
		case 'w': //write
		for (tracker=0; tracker != MAX; tracker++) {
			eeprom_write_word ( (uint16_t*)address, *(array+tracker) );
			address = address + sizeof( (*array+tracker) );
		}
		
		default:
		return 0;
	}
}
	
void begin_saving_process (int no_of_iterations, int array_size, uint16_t* array, uint16_t address) {
	
	saving_on;
	
	int start_max = 0;
	int start_min = 1023;
	int tracker = 0;
	volatile uint16_t temp;
	
	*(array+MAX) = 1; //We don't want to have a negative number when subtraction happens the first time, do we?
	*(array+MIN) = 1;
	
	do {
		/* Let's say the device took readings when a light was shone on it,
		 * and then the max value goes high. We know this highest isn't authentic.
		 * To correct that, max reduces by 1, min increases by 1
		 */   
		*(array+MAX) = *(array+MAX) - 1;
		*(array+MIN) = *(array+MIN) + 1;
		
		temp = Read_from_ADC( ch ); //Read analog value from ADC.
		if (temp > start_max) {
			*(array+MAX) = temp;
		}
		else if (temp < start_min) {
			*(array+MIN) = temp;
		}
		tracker++;
		
	} while (tracker < no_of_iterations);
	
	
	/* No harm if the maximum and minimum values are the same */
	*(array+NOW) = ( *(array+MAX) + *(array+MIN) ) / 2;
	
	 EEPROM_handler('w', address, array); //int EEPROM_handler( char mode, uint16_t address, uint16_t array ) 
	 
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
	
	for(tracker=0, total=0; tracker < number_of_iterations; tracker++) {
		total = total - Readings[array_index];
		Readings[array_index] = Read_from_ADC(ch);
		total = total + Readings[array_index];
		array_index++;
		if (array_index >= array_size) {
			array_index = 0;
		}
	average = total / array_size;
	return average;
	}
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

/* I copied everything below. */

void Initialize_ADC( void )
{
	ADMUX=(1<<REFS0); // For Aref=AVcc;
	ADCSRA=(1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0); //Prescalar div factor =128
}

uint16_t Read_from_ADC (uint8_t channel) //Reading from Channel zero
{
	//Select ADC Channel ch must be 0-7
	channel = channel & 0b00000111;
	ADMUX |= channel;

	//Start Single conversion
	ADCSRA |= (1<<ADSC);

	//Wait for conversion to complete
	while( !(ADCSRA & (1<<ADIF) ) );
	
	//Clear ADIF by writing one to it
	//Note you may be wondering why we have write one to clear it
	//This is standard way of clearing bits in io as said in datasheets.
	//The code writes '1' but it result in setting bit to '0' !!!
	ADCSRA|=(1<<ADIF);
	
	return(ADC);
}

