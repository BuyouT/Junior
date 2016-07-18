#include <msp430.h> 

/*
 * Author: Stefano Reyna 
 * LED blink and sound
 */
int i;

void delay_us(unsigned int us)
{
    unsigned int i;
    for (i = 0; i<= us/2; i++)
       __delay_cycles(1);
}

void beep(unsigned int note)
{
    long delay = (long)(10000/note);  //Note's semiperiod.
    P1OUT |= 0x01;     //Set P1
    delay_us(delay);   //play half
    P1OUT &= ~0x01;    //reset
    delay_us(delay);   //play half
}

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	P2DIR = 0x01; //Set output to P2
	P1DIR = 0x01; //Set output to P1
	P3DIR = 0x00; //Set input to P3
	P2OUT = 0x00;
	while(1) {
		//LED blink 2hz
		if (P3IN & 0x01) {
			P2OUT ^= 0x01;
			for(i = 0; i < 20500; i++); //Trial and error with the o-scope
		}
		//sound 2khz
		else {
			beep(255); //Trial and error with the o-scope
		}
	}
	return 0;
}
