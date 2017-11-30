#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <avr/interrupt.h>

#include "lcd.h"
#include "adc.h"

volatile int qsecs;
volatile int tens = 0;
volatile int ones = 0;
volatile int tenths = 0;


int main(void)
{
    int clear = 0;
    int state = 0; //stopped;
    unsigned char adc_result;
    int otens = 0;
    int oones = 0;
    int otenths = 0;
    // Initialize the LCD
    lcd_init();

    // Initialize the ADC
    adc_init(0);
    

    // Write splash screen
    lcd_writecommand(1);
    char name[17];
    char* steph = "Stephanie Lampotang";
    snprintf(name, 17, "%s", steph);
    lcd_stringout(name);
    _delay_ms(2000);
    
    //set starting screen
    lcd_writecommand(1);
    lcd_writedata(tens + 0x30);
    lcd_writedata(ones + 0x30);
    lcd_writedata('.');
    lcd_writedata(tenths + 0x30);
    
    //Initializing registers
    TCCR1B |= (1 << WGM12);
    TIMSK1 |= (1 << OCIE1A);
    OCR1A = 6250;
    TCCR1B |= (1 << CS12);
    sei();

    while (1)
    {                 // Loop forever
    // Do a conversion
//        char buf[5];
//        adc_result = adc_sample();
//        snprintf(buf, 5, "%4d", adc_result);
//        lcd_moveto(0,0);
//        lcd_stringout(buf);
//        //top = 51 r = 0 b = 101 l = 156

        if(state == 0) {
            //turn of interrupts
            TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10));
            // Handle inputs for state 0
            //start/stop button (top button)
            if((adc_result > 46) && (adc_result < 56))
            {
                while((adc_sample() > 46) && (adc_sample() < 56))
                {}
                state = 1;
            }
            //lap/reset button (bottom button)
            else if((adc_result > 96) && (adc_result < 106))
            {
                while((adc_sample() > 96) && (adc_sample() < 106))
                {}
                tens = 0;
                ones = 0;
                tenths = 0;
                qsecs = 1;
                state = 0;
            }
        }
        else if(state == 1) {
            //turn interrupts back on
            TCCR1B |= (1 << CS12);
            // Handle inputs for state 1
            if((adc_result > 46) && (adc_result < 56))
            {
                while((adc_sample() > 46) && (adc_sample() < 56))
                {}
                state = 0;
            }
            else if((adc_result > 96) && (adc_result < 106))
            {
                while((adc_sample() > 96) && (adc_sample() < 106))
                {}
                state = 2;
            }
        }
        else if(state == 2)
        {
            // Handle inputs for state 2
            if((adc_result > 46) && (adc_result < 56))
            {
                while((adc_sample() > 46) && (adc_sample() < 56))
                {}
                state = 1;
            }
            else if((adc_result > 96) && (adc_result < 106))
            {
                while((adc_sample() > 96) && (adc_sample() < 106))
                {}
                state = 1;
            }
        }
        

        
            
            //change the display if the state is 1 or if we want to clear it from state 0
            if(((state == 1) && (qsecs == 1)) || ((state == 0) && (clear = 1)))
            {
                if(otenths != tenths)
                {
                    lcd_moveto(0,3);
                    lcd_writedata(tenths + 0x30);
                    otenths = tenths;
                }
                if(oones != ones)
                {
                    lcd_moveto(0,1);
                    lcd_writedata(ones + 0x30);
                    oones = ones;
                }
                if(otens != tens)
                {
                    lcd_moveto(0,0);
                    lcd_writedata(tens + 0x30);
                    otens = tens;
                }
            }
            qsecs = 0;
        
    }

    return 0;   /* never reached */
}

ISR(TIMER1_COMPA_vect) {
    //flag
    qsecs++;
    //logic for counting upwards with three separate variables for the time
    if(tenths == 9)
    {
        tenths = 0;
        if(ones != 9)
        {
            ones = ones+1;
        }
        else {
            ones = 0;
            if(tens < 5)
            {
                tens = tens+1;
            }
            else {
                tens = 0;
            }
        }
    }
    else {
        tenths = tenths+1;
    }
}

