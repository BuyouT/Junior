#include <msp430.h> 

/*
 * Author: Stefano Reyna 
 * Ohm meter
 */
#define HOME	0x02	// Home
#define CLEAR	0x01	// Clear screen and CR

void lcd_command(char);
void lcd_char(char);
void lcd_init(void);
void lcd_string(char*);
void adc_setup(void);
unsigned int adc_sam20(void);
void getResistance(unsigned int);
char uf_lcd_temp;
char uf_lcd_temp2;
char uf_lcd_x;
char ohm = 222;

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
    lcd_init();
    adc_setup();
    

    while (1) {
    	unsigned int avg_adc = 0;
    	avg_adc = adc_sam20();		// sample 20 times and average out the result
    	getResistance(avg_adc);		// print out the result to the screen
    }

	return 0;
}

void lcd_string(char *str) {
	//This writes words yo
	while (*str != 0) {
		lcd_char(*str);
		*str++;
	}
}

void lcd_command(char uf_lcd_x){
	P4DIR = 0xFF;
	uf_lcd_temp = uf_lcd_x;
	P4OUT = 0x00;
	__delay_cycles(22000);
	uf_lcd_x = uf_lcd_x >> 4;
	uf_lcd_x = uf_lcd_x & 0x0F;
	uf_lcd_x = uf_lcd_x | 0x20;
	P4OUT = uf_lcd_x;
	__delay_cycles(22000);
	uf_lcd_x = uf_lcd_x & 0x0F;
	P4OUT = uf_lcd_x;
	__delay_cycles(22000);
	P4OUT = 0x00;
	__delay_cycles(22000);
	uf_lcd_x = uf_lcd_temp;
	uf_lcd_x = uf_lcd_x & 0x0F;
	uf_lcd_x = uf_lcd_x | 0x20;
	P4OUT = uf_lcd_x;
	__delay_cycles(22000);
	uf_lcd_x = uf_lcd_x & 0x0F;
	P4OUT = uf_lcd_x;
	__delay_cycles(22000);
}

void lcd_char(char uf_lcd_x){
	P4DIR = 0xFF;
	uf_lcd_temp = uf_lcd_x;
	P4OUT = 0x10;
	__delay_cycles(22000);
	uf_lcd_x = uf_lcd_x >> 4;
	uf_lcd_x = uf_lcd_x & 0x0F;
	uf_lcd_x = uf_lcd_x | 0x30;
	P4OUT = uf_lcd_x;
	__delay_cycles(22000);
	uf_lcd_x = uf_lcd_x & 0x1F;
	P4OUT = uf_lcd_x;
	__delay_cycles(22000);
	P4OUT = 0x10;
	__delay_cycles(22000);
	uf_lcd_x = uf_lcd_temp;
	uf_lcd_x = uf_lcd_x & 0x0F;
	uf_lcd_x = uf_lcd_x | 0x30;
	P4OUT = uf_lcd_x;
	__delay_cycles(22000);
	uf_lcd_x = uf_lcd_x & 0x1F;
	P4OUT = uf_lcd_x;
	__delay_cycles(22000);
}

void lcd_init(void){
	lcd_command(0x33);
	lcd_command(0x32);
	lcd_command(0x2C);
	lcd_command(0x0C);
	lcd_command(0x01);
}

// ADC10 interrupt service routine
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void) {
	__bic_SR_register_on_exit(CPUOFF);        // Return to active mode
}

void adc_setup(void) {
	// ADC
	ADC10CTL0 = ADC10SHT_2 + ADC10ON + ADC10IE; //0x1018, 16xADC10CLks, ADC10 on, and ADC10 interupt enable
	ADC10AE0 |= 0x01; //Enable reg 0
}

unsigned int adc_sam20(void) {
	unsigned int i;
	volatile unsigned int sum = 0;
	volatile unsigned int value;
	for(i = 0; i < 20; i++)
	{
		ADC10CTL0 |= ENC + ADC10SC;             // Start the conversion and enable conversion
		__bis_SR_register(CPUOFF + GIE);        // call ISR, low power mode0
		value = ADC10MEM;						// Save measured value
		sum += value;
	}
	return sum / 20;							// Average result
}

void getResistance(unsigned int adcIn) {
	static unsigned short flag = 0;
	volatile float voltage = ((float)adcIn/1023)*3.3; //average*(accuracy = span(3.3V)/resolution(1024)), uP is from 0-1023
	float resistance = 10000*(5/voltage-10); //using voltage divider, 10k as a reference resistor

	if (adcIn > 930 || adcIn < 10) {
		if(flag == 0) {
			lcd_command(CLEAR);
			flag = 1;
		}
		lcd_string("Out of Range");
		lcd_command(HOME);
	}
	else {
		flag = 0;
		unsigned long intRes = resistance;
		int numArray[7];
		int i;
		char numToChar;
		lcd_string("R = ");

		for(i = 6 ;i >= 0; i--) {
			numArray[i] = intRes % 10;  //get last digit
			intRes /= 10;				//make number smaller
		}

		for(i = 0; i < 7; i++) {
			numToChar = numArray[i] +'0'; //atoi pretty much
			lcd_char(numToChar);
		}
		lcd_char(' ');
		lcd_char(ohm);
		lcd_command(HOME);
	}
}
