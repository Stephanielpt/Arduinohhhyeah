#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#include "lcd.h"
#include "adc.h"

void serial_init(unsigned short);
void serial_stringout(char *);
void serial_txchar(char);
volatile int incoming = 0;
int place = 0;
char heyyy[5];


volatile int flagger;
volatile int qsecs;
volatile unsigned char maxstate = 0;
volatile int maxi;        // Count to display
volatile unsigned char a, b;
volatile unsigned char status;
volatile int maxx = 0;
volatile int count = 0;
volatile int count1 = 0;

#define FOSC 16000000           // Clock frequency
#define BAUD 9600               // Baud rate used
#define MYUBRR FOSC/16/BAUD-1   // Value for UBRR0 register
#define ADC_CHAN 0

int main(void)
{
    maxi = eeprom_read_byte((void *) 100);
    if(maxi < 1 || maxi > 99)
    {
        maxi = 1;
    }
    int state = 0; //not timing;
    
    //Initialize serial
    UBRR0 = MYUBRR;             // Set baud rate
    serial_init(MYUBRR);
    UCSR0B |= (1 << RXCIE0);
    
    // Initialize the LCD
    lcd_init();
    
    // Write splash screen
    lcd_writecommand(1);
    char time[17];
    char view[17];
    char* milli = "ms = ";
    char name[17];
    char* maxclear = "Max=            ";
    char* steph = "Stephanie Lampotang";
    snprintf(name, 17, "%s", steph);
    lcd_stringout(name);
    _delay_ms(2000);
    lcd_writecommand(1);
    lcd_moveto(1,0);
    char bottomline[17];
    char* maxSpeed = "Max=";
    
    snprintf(bottomline, 17, "%s%d", maxSpeed, maxi);
    lcd_stringout(bottomline);
    
    //set up states for rotary encoder
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
    
    //Initializing registers
    TCCR1B |= (1 << WGM12);
    TIMSK1 |= (1 << OCIE1A);
    OCR1A = 16000;
    sei();
    //set up a timer for the buzzer so...
    //if speed > maxi then turn the time counter on and
    //interrupt every so often (frequency) which will:
    //turn portb on and off
    //increment a count so that the buzz lasts a while
    //if the count hits like a second or whatever
    //turn timer off
    TCCR0A |= (1 << WGM01);
    TIMSK0 |= (1 << OCIE0A);
    OCR0A = 36362/2;
    
    //Initializing inputs n outputs
    DDRD &= ~(1 << 2);
    DDRD &= ~(1 << 3);
    PORTD |= (1 << 2);
    PORTD |= (1 << 3);
    DDRC |= (1 << 1);
    PORTC |= (1 << PC2) | (1 << PC3);
    DDRB |= (1 << 4);
    
    //Enabling PortD interrupts
    PCICR |= (1 << PCIE2);
    PCMSK2 |= (1 << 2);
    PCMSK2 |= (1 << 3);
    
    //Enabling PortC interrupts
    PCICR |= (1 << 1);
    PCMSK1 |= (1 << 2);
    PCMSK1 |= (1 << 3);
    
    int oldstate = 0;
    int dec;
    int full;
    int speed;
    
    while (1)
    {                 // Loop forever
        //change rotary encoder max speed
        if(maxx != maxi)
        {
            lcd_moveto(1,0);
            snprintf(bottomline, 17, "%s", maxclear);
            lcd_stringout(bottomline);
            lcd_moveto(1,0);
            snprintf(bottomline, 17, "%s%d", maxSpeed, maxi);
            lcd_stringout(bottomline);
            maxx = maxi;
            eeprom_update_byte((void *) 100, maxi);
        }
        if(flagger == 1)
        {
            if((PIND & (1<<2)) == 0)
            {
                state = 1; //timing
                if(oldstate != state) //update necessary?
                {
                    PORTC |= (1 << 1);
                    //turn timer on
                    TCCR1B |= (1 << CS10);
                    //reset oldstate
                    oldstate = state;
                }
            }
            
            else if((PIND & (1<<3)) == 0)
            {
                state = 0; //stop timer
                if(oldstate != state) //update necessary?
                {
                    PORTC &= ~(1 << 1);
                    //reset oldstate
                    oldstate = state;
                    //turntimer off
                    TCCR1B &= ~(1 << CS10);
                    //calculate speed and print it out after total time
                    unsigned int cm = 1.3*2.54*10*1000;
                    speed = cm/count;
                    dec = speed%10;
                    full = speed/10;
                    lcd_moveto(0,2);
                    snprintf(time, 17, "%d%s%d%s%d%s", count, milli, full, ".", dec, "  ");
                    lcd_stringout(time);
                    //print out the speed transmitted by serial
                    snprintf(time, 6, "%s%d%s", "@", speed, "$");
                    serial_stringout(time);
                    //reset count;
                    count = 0;
                }
            }
            //reset flagger;
            flagger = 0;
        }
        
        if(incoming == 1) //a new message has been received
        {
            //print incoming message
            lcd_moveto(1,10);
            int getback;
            sscanf(heyyy, "%d", &getback);
            
            int big = getback/10;
            int lil = getback%10;
            //            snprintf(view, 17, "%d|%s|%d.%d  ", speed, heyyy, big, lil);
            snprintf(view, 17, "%d.%d  ", big, lil);
            lcd_stringout(view);
            //if the speed was too fast - set buzzer timer on
            if(getback/10 >= maxi)
            {
                TCCR0B |= (1 << CS00);
            }
            incoming = 0;
        }
    }
    return 0;   /* never reached */
}

ISR(TIMER0_COMPA_vect) {
    _delay_ms(1);
    if(count1 < 440)
    {
        if(count1%2 == 0)
        {
            PORTB |= (1 << PB4);
            count1++;
        }
        else if(count1%2 == 1)
        {
            PORTB &= ~(1 << PB4);
            count1++;
        }
    }
    else {
        TCCR0B &= ~(1 << CS00);
        count1 = 0;
    }
}

ISR(TIMER1_COMPA_vect) {
    count++;
}

ISR(PCINT2_vect)
{
    flagger = 1;
}

ISR(PCINT1_vect)
{
    //Read the input bits and determine a and b
    status = PINC;
    a = status & (1 << 2); //MIGHT NEED TO CHANGE
    b = status & (1 << 3); //MIGHT NEED TO CHANGE
    
    if (maxstate == 0) {
        
        // Handle A and B inputs for state 0
        if(b && !a)
        {
            maxstate = 3;
            if(maxi > 1)
            {
                maxi--;
            }
        }
        if(!b && a)
        {
            maxstate = 1;
            if(maxi < 99)
            {
                maxi++;
            }
        }
    }
    else if (maxstate == 1) {
        
        // Handle A and B inputs for state 1
        if(!b && !a)
        {
            maxstate = 0;
            if(maxi > 1)
            {
                maxi--;
            }
        }
        if(b && a)
        {
            maxstate = 2;
            if(maxi < 99)
            {
                maxi++;
            }
        }
        
    }
    else if (maxstate == 2) {
        
        // Handle A and B inputs for state 2
        if(!b && a)
        {
            maxstate = 1;
            if(maxi > 1)
            {
                maxi--;
            }
        }
        if(b && !a)
        {
            maxstate = 3;
            if(maxi < 99)
            {
                maxi++;
            }
        }
    }
    else if (maxstate == 3){
        
        // Handle A and B inputs for state 3
        if(b && a)
        {
            maxstate = 2;
            if(maxi > 1)
            {
                maxi--;
            }
        }
        if(!b && !a)
        {
            maxstate = 0;
            if(maxi < 99)
            {
                maxi++;
            }
        }
    }
}

//----------------------------------------------------------------------

void serial_init(unsigned short baud)
{
    // Set up USART0 registers
    UCSR0C = (3 << UCSZ00);               // Async., no parity,
    // 1 stop bit, 8 data bits
    UCSR0B |= (1 << TXEN0 | 1 << RXEN0);  // Enable RX and TX
    // Enable tri-state
    DDRB |= (1<<3);
    PORTB &= ~(1<<3);
    
}

void serial_txchar(char ch)
{
    while ((UCSR0A & (1<<UDRE0)) == 0) {}
    UDR0 = ch;
}

void serial_stringout(char *s)
{
    // Call serial_txchar in loop to send a string
    int i = 0;
    if(s[i] == '@')
    {
        i++;
        while(s[i] != '$')
        {
            serial_txchar(s[i]);
            i++;
        }
        serial_txchar(s[i]);
    }
}

ISR(USART_RX_vect)
{
    char ch;
    ch = UDR0;                  // Get the received charater
    // Store in buffer
    if(ch != '$')
    {
        heyyy[place] = ch;
        place++;
    }
    // If message complete, set flag
    else
    {
        while(place < 4)
        {
            heyyy[place] = 0;
            place++;
        }
        place = 0;
        incoming = 1;
    }
}


