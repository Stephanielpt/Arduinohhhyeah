/********************************************
*
*  Name: Stephanie Lampotang
*  Section: Redekopp
*  Assignment: Lab 4 - Up/Down counter on LCD display
*
********************************************/

#include <avr/io.h>
#include <util/delay.h>

#include "lcd.h"

int main(void) {

        // Setup DDR and PORT bits for the 2 input buttons
        // as necessary

        // Initialize the LCD
    lcd_init();
    
        // Use a state machine approach to organize your code
        //   - Declare and initialize a variable to 
        //     track what state you are in
    # define maskbits 0xeb

    DDRC &= maskbits;
    PORTC |= ~(maskbits);
	char state = 0;
    char count  = 0;

    while (1) {               // Loop forever
    int i;
	for(i=0; i < 50; i++)
	{
		if((PINC | ~(1 << 4)) == ~(1 << 4))
		{
			state = 0;
		}
		if((PINC | ~(1 << 2)) == ~(1 << 2))
		{
			state = 1;
		}
		i++;
		_delay_ms(20);
	}
	if(state == 0)
	{
		lcd_writecommand(1);
		lcd_writedata(count + 0x30);
        if(count == 9)
        {
            count = -1;
        }
		count++;
	}

	if(state == 1)
	{
		lcd_writecommand(1);
		lcd_writedata(count + 0x30);
        if(count == 0)
        {
            count = 10;
        }
		count--;
		

                // Then inside each 'if' or 'else' block, perform the desired
                //  output operations and then sample the inputs to update
                //  the state for the next iteration
        
                
                // Delay before we go to the next iteration of the loop
                
    }
    }
    return 0;   /* never reached */
}
