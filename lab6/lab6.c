#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>

#include "lcd.h"
#include "adc.h"
#include "adc.c"



int main(void)
{
    unsigned char adc_result;

    // Initialize the LCD
    lcd_init();

    // Initialize the ADC
    adc_init(3);

    // Use the ADC to find a seed for rand();
    adc_result = adc_sample();
    srand(adc_result << 8 | adc_result);  // Make a 16-bit number for srand()

    // Write splash screen
    lcd_writecommand(1);
    char name[17];
    char* steph = "Stephanie Lampotang";
    snprintf(name, 17, "%s", steph);
    lcd_stringout(name);
    _delay_ms(2000);

    lcd_writecommand(1);

    // Find a random initial position of the 'X'
    int position = rand()%16;
    
    // Display the 'X' on the screen
    lcd_moveto(0, position);
    lcd_writedata('X');
    
    int oldArrow = adc_result;
    oldArrow = oldArrow/16;
    lcd_moveto(1, oldArrow);
    lcd_writedata('^');

    while (1) 
    {                 // Loop forever
    // Do a conversion
    
    int arrow = adc_sample();
        
    arrow = arrow/16;
    
	// Move '^' to new position
    //check if we need to update the screen
    if(arrow != oldArrow) 
    {
        //erase old arrow
        lcd_moveto(1, oldArrow);
        lcd_writedata(' ');
        //write new arrow
        lcd_moveto(1, arrow);
        lcd_writedata('^');
        //reset the value of the arrow
        oldArrow = arrow;
        _delay_ms(10);
    }
    

	// Check if '^' is aligned with 'X'
    if(position == arrow)
    {
        int hold = adc_sample();
        int time = 0;
        while(position == arrow)
        {
            //see if the knob has been turned
            if(adc_sample() != hold)
            {
                break;
            }
            //otherwise begin counting time
            _delay_ms(10);
            time++;
            //if time elapsed is 2 seconds you win
            if(time >= 200)
            {
                lcd_writecommand(1);
                lcd_stringout("You win!");
                return 0;
            }
        }
    }



    }

    return 0;   /* never reached */
}

