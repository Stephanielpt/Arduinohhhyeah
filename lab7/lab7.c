/********************************************
 *
 *  Name:
 *  Section:
 *  Assignment: Lab 7 - Rotary Encoder
 *
 ********************************************/

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>

#include "lcd.h"

void variable_delay_us(int);
void play_note(unsigned short);

// Frequencies for natural notes from middle C (C4)
// up one octave to C5.
unsigned short frequency[8] =
    { 262, 294, 330, 349, 392, 440, 494, 523 };

volatile unsigned char state;
volatile signed int count = 0;		// Count to display
volatile unsigned char a, b;
volatile unsigned char status;
volatile int old_count = 0;

int main(void) {
    
    // Initialize DDR and PORT registers and LCD
    DDRC &= ~(1 << 5);
    DDRC &= ~(1 << 1);
    PORTC |= (0x22);
    DDRB |= (1 << 4);
    PCICR |= (1 << 1);
    PCMSK1 |= (1 << 1);
    PCMSK1 |= (1 << 5);
    sei();
    lcd_init();
    // Write a spash screen to the LCD
    lcd_writecommand(1);
    char name[17];
    char* steph = "Stephanie :) Lampotang";
    snprintf(name, 17, "%s", steph);
    lcd_stringout(name);
    _delay_ms(1000);
    
    lcd_writecommand(1);

    // Read the A and B inputs to determine the intial state
    // Warning: Do NOT read A and B separately.  You should read BOTH inputs
    // at the same time, then determine the A and B values from that value.    
    status = PINC;
    a = status & (1 << 5);
    b = status & (1 << 1);
    //initial state
    if (!b && !a)
	state = 0;
    else if (!b && a)
	state = 1;
    else if (b && !a)
	state = 2;
    else // (b && a)
	state = 3;
    
    
    while (1) {                 // Loop forever

        if (old_count != count) { // Did state change?

	    // Output count to LCD
            lcd_writecommand(1);
            lcd_moveto(0,0);
            char output[17];
            snprintf(output, 17, "%d", count);
            lcd_stringout(output);
            

	     //Do we play a note?
            int temp = count;
            if(temp < 0)
            {
                temp = 0 - temp;
            }
	    if ((((temp) % 8) == 0) && (old_count != count)) {
		// Determine which note (0-7) to play
            int whichNote = temp/8;
            if(whichNote > 7)
            {
                whichNote = whichNote % 8;
            }
		// Find the frequency of the note
            int theNote = frequency[whichNote];
		// Call play_note and pass it the frequency
            play_note(theNote);
            
	    }
            old_count = count;
        }
    }
}

ISR(PCINT1_vect)
{
    //Read the input bits and determine a and b
    status = PINC;
    a = status & (1 << 5);
    b = status & (1 << 1);
    
    if (state == 0) {
        
        // Handle A and B inputs for state 0
        if(b && !a)
        {
            state = 3;
            count--;
        }
        if(!b && a)
        {
            state = 1;
            count++;
        }
    }
    else if (state == 1) {
        
        // Handle A and B inputs for state 1
        if(!b && !a)
        {
            state = 0;
            count--;
        }
        if(b && a)
        {
            state = 2;
            count++;
        }
        
    }
    else if (state == 2) {
        
        // Handle A and B inputs for state 2
        if(!b && a)
        {
            state = 1;
            count--;
        }
        if(b && !a)
        {
            state = 3;
            count++;
        }
    }
    else if (state == 3){
        
        // Handle A and B inputs for state 3
        if(b && a)
        {
            state = 2;
            count--;
        }
        if(!b && !a)
        {
            state = 0;
            count++;
        }
    }

}
/*
  Play a tone at the frequency specified for one second
*/
void play_note(unsigned short freq)
{
    unsigned long period;

    period = 1000000 / freq;      // Period of note in microseconds

    while (freq--) {
	// Make PB4 high
        PORTB |= (1 << PB4);
	// Use variable_delay_us to delay for half the period
        variable_delay_us(period/2);
	// Make PB4 low
        PORTB &= ~(1 << PB4);
	// Delay for half the period again
        variable_delay_us(period/2);
    }
}

/*
    variable_delay_us - Delay a variable number of microseconds
*/
void variable_delay_us(int delay)
{
    int i = (delay + 5) / 10;

    while (i--)
        _delay_us(10);
}

