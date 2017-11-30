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
#include <stdio.h>

int main(void) {

        // Setup DDR and PORT bits for the 2 input buttons
        // as necessary

        // Initialize the LCD
    lcd_init();
    
        // Use a state machine approach to organize your code
        //   - Declare and initialize a variable to 
        //     track what state you are in
    //display splash screen
    lcd_writecommand(1);
    char name[17];
    char* steph = "Stephanie Lampotang";
    snprintf(name, 17, "%s", steph);
    lcd_stringout(name);
    //center birth date
    lcd_moveto(1, 1);
    char birthday[17];
    char day;
    int year;
    char *month = "August";
    day = 4;
    year = 1999;
    snprintf(birthday, 17, "%s %d, %d", month, day, year);
    lcd_stringout(birthday);
    //wait before running main function
    _delay_ms(2000);
    //clear screen for main function
    lcd_writecommand(1);
    
    # define maskbits 0xeb
    
    DDRC &= maskbits;
    PORTC |= ~(maskbits);
    
    char state = 0;
    char *status = "Locked";
    
    //checks if button B is pressed and if it is, returns true
    int buttonB()
    {
        if((PINC | ~(1 << 4)) == ~(1 << 4))
        {
            //delays during the bouncing
            _delay_ms(5);
            //accounts for length of press
            while((PINC | ~(1 << 4)) == ~(1 << 4)) {}
            //delays during the bouncing
            _delay_ms(5);
            return 1;
        }
        else {
            return 0;
        }
    }
    
    //checks if button A is pressed and if it is, returns true
    int buttonA()
    {
        if((PINC | ~(1 << 2)) == ~(1 << 2))
        {
            //delays during the bouncing
            _delay_ms(5);
            //accounts for length of press
            while((PINC | ~(1 << 2)) == ~(1 << 2)) {}
            //delays during the bouncing
            _delay_ms(5);
            return 1;
        }
        else {
            return 0;
        }
    }

    while (1)
    {               // Loop forever
        //print opening screen
        char firstline[17];
        snprintf(firstline, 17, "State = S%d", state);
        lcd_stringout(firstline);
        lcd_moveto(1, 0);
        char secondline[17];
        snprintf(secondline, 17, "Status= %s", status);
        lcd_stringout(secondline);
        lcd_moveto(0,0);
        //logical state diagram
        if(state == 0)
        {
            if(buttonB())
            {
                //B is the correct 1st entry so progress to next state
                state = 1;
            }
        }
        else if(state == 1)
        {
            if(buttonA())
            {
                //A is the correct 1st entry so progress to next state
                state = 2;
            }
            if(buttonB())
            {
                //B is not correct but could be the beginning of a new sequence
                state = 1;
            }
        }
        else if(state == 2)
        {
            if(buttonA())
            {
                //A is the correct 1st entry so progress to next state
                state = 3;
            }
            if(buttonB())
            {
                //B is not correct but could be the beginning of a new sequence
                state = 1;
            }
        }
        else if(state == 3)
        {
            if(buttonA())
            {
                //A is not correct - reset
                state = 0;
            }
            if(buttonB())
            {
                //B is the correct 1st entry so progress to next state
                state = 4;
            }
        }
        else if(state == 4)
        {
            status = "Unlocked";
            //regardless of press, the status is locked and state returns to 0
            if(buttonA())
            {
                state = 0;
                status = "Locked  ";
            }
            if(buttonB())
            {
                state = 0;
                status = "Locked  ";
            }
        }
        
        
        
            }
    return 0;   /* never reached */
}
