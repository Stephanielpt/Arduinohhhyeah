/********************************************
 *
 *  Name:
 *  Section:
 *  Assignment: Lab 9 - Serial Communications
 *
 ********************************************/

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>

#include "lcd.h"
#include "adc.h"

// Serial communications functions and variables
void serial_init(unsigned short);
void serial_stringout(char *);
void serial_txchar(char);
volatile int incoming = 0;
int place = 0;
char heyyy[17];

#define FOSC 16000000           // Clock frequency
#define BAUD 9600               // Baud rate used
#define MYUBRR FOSC/16/BAUD-1   // Value for UBRR0 register
#define ADC_CHAN 0

// ADC functions and variables
void adc_init(unsigned char);

char *messages[] = {    //all must start with 'h'
    "Hello           ",
    "How are you?    ",
    "ho ho ho        ",
    "Hambaga!        ",
    "Hasta luego     ",
    "Hark            ",
    "heart to heart  ",
    "home dawg       ",
    "Havent you heard",
    "Horrible!       ",
};

int main(void) {
    UBRR0 = MYUBRR;             // Set baud rate

    int oldView = 0; //to check if the lcd should update
    int viewable = 0;
    unsigned char adc_result;
    char view[17];
    char* texts;

    // Initialize the modules and LCD
    lcd_init();
    adc_init(ADC_CHAN);
    serial_init(MYUBRR);

    // Enable interrupts
    UCSR0B |= (1 << RXCIE0);    // Enable receiver interrupts
    sei();                      // Enable interrupts

    // Show the splash screen
    lcd_writecommand(1);
    char name[17];
    char* steph = "Stephanie :) Lampotang";
    snprintf(name, 17, "%s", steph);
    lcd_stringout(name);
    _delay_ms(1000);
    
    lcd_writecommand(1);
    
    //screen displays first of possible messages
    texts = messages[viewable];
    snprintf(view, 17, "%s", texts);
    lcd_moveto(0,0);
    lcd_stringout(view);
    
    while (1) {    // Loop forever
        //check if the lcd should update
        if(oldView != viewable)
        {
            //update with the new message option
            texts = messages[viewable];
            snprintf(view, 17, "%s", texts);
            lcd_moveto(0,0);
            lcd_stringout(view);
            oldView = viewable;
        }
        
        if(incoming == 1) //a new message has been received
        {
            //print incoming message
            lcd_moveto(1,0);
            snprintf(view, 17, "%s", heyyy);
            lcd_stringout(view);
            incoming = 0;
        }
        // Get an ADC sample
        adc_result = adc_sample();
    
        // Up button pressed?
        if((adc_result > 46) && (adc_result < 56))
        {
            while((adc_sample() > 46) && (adc_sample() < 56))
            {
                //wait for button to be let go of
            }
            //allow circular loop
            if(viewable == 0)
            {
                viewable = 9;
            }
            else if(viewable > 0)
            {
                viewable--;
            }
        }

        // Down button pressed?
        else if((adc_result > 96) && (adc_result < 106))
        {
            while((adc_sample() > 96) && (adc_sample() < 106))
            {
                //wait for button to be let go of
            }
            if(viewable < 9)
            {
                viewable++;
            }
            //allow circular loop
            else if(viewable == 9)
            {
                viewable = 0;
            }
        }
        // Select button pressed?

        else if((adc_result > 200) && (adc_result < 210))
        {
            while((adc_sample() > 200) && (adc_sample() < 210))
            {
                //wait for button to be let go of
            }
            serial_stringout(messages[viewable]);
        }
        // Message received from remote device?
    }
}

/* ----------------------------------------------------------------------- */

void serial_init(unsigned short baud)
{
    // Set up USART0 registers
    UCSR0C = (3 << UCSZ00);               // Async., no parity,
    // 1 stop bit, 8 data bits
    UCSR0B |= (1 << TXEN0 | 1 << RXEN0);  // Enable RX and TX
    // Enable tri-state
    DDRD |= (1<<3);
    PORTD &= ~(1<<3);
}

void serial_txchar(char ch)
{
    while ((UCSR0A & (1<<UDRE0)) == 0)
    UDR0 = ch;
}

void serial_stringout(char *s)
{
    // Call serial_txchar in loop to send a string
    for(int i = 0; i < 16; i++)
    {
        serial_txchar(s[i]);
    }
}

ISR(USART_RX_vect)
{
    char ch;
    ch = UDR0;                  // Get the received charater
    
    // Store in buffer
    heyyy[place] = ch;
    place++;
    if(place == 16)
    {
        heyyy[place] = 0;
        place = 17;
    }
    // If message complete, set flag
    if(place == 17)
    {
        place = 0;
        incoming = 1;
    }
}
