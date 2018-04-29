#define _XTAL_FREQ 24000000
#pragma config WDT = OFF
#pragma config XINST = OFF
#pragma config OSC  = HS
#pragma config PWRT = OFF
#pragma config LVP = ON

#include <xc.h>
#include <stdio.h>

void initUART1()
{
    TRISCbits.TRISC6 = 0;   // TX as output
    TRISCbits.TRISC7 = 1;   // RX as input

    TXSTA1bits.SYNC = 0;    // async operation
    TXSTA1bits.TX9  = 0;    // 8-bit data
    TXSTA1bits.TXEN = 1;    // enable transmitter

    RCSTA1bits.RX9  = 0;    // 8-bit data
    RCSTA1bits.CREN = 1;    // enable receiver
    RCSTA1bits.SPEN = 1;    // enable serial port

    BAUDCON1bits.BRG16 = 0; // 8-bit divisor
    TXSTA1bits.BRGH = 0;    // low speed UART

    // baud rate in this mode is:
    // FOSC/(64*(n+1))
    //
    // where n = SPBRG1

    SPBRG1 = 38;    // 9615.38 baud

}

void putchar(char c)
{
    while(TXSTA1bits.TRMT ==0) {};  // wait for transmission to end..
    TXREG1 = c;
}

void main() 
{
    initUART1();

    while(1)
    {
        printf("Hello, world!\n\r");
    };
}

