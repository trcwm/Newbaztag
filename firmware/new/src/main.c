#define _XTAL_FREQ 24000000
#pragma config WDT = OFF
#pragma config XINST = OFF
#pragma config OSC  = HS
#pragma config PWRT = OFF
#pragma config LVP = ON

#include <xc.h>
#include <stdio.h>

// ************************************************************
// EUSART
// ************************************************************

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


char getchar()
{
    while (PIR1bits.RC1IF == 0)
    {
        if (RCSTA1bits.OERR == 1)
        {
            RCSTA1bits.OERR = 0;    // clear overrun if necessary
            RCSTA1bits.CREN = 0;    // reset RX part of UART
            RCSTA1bits.CREN = 1;
        }
    }
    return RCREG1;
}


// ************************************************************
// SPI
// ************************************************************


typedef enum 
{
    SEL_NONE = 0,   // no device selected (or LEDS)
    SEL_FLASH,      // select flash
    SEL_AUDIO       // select OKI chip
} spiSlave_t;


void selectSPISlave(spiSlave_t slaveID)
{
    switch(slaveID)
    {
    default:        
    case SEL_NONE:
        PORTBbits.RB3 = 1;  // disable MEM
        PORTAbits.RA2 = 1;  // disable AUDIO
        PORTBbits.RB5 = 0;  // XLAT_LED = 0
        break;
    case SEL_FLASH:
        PORTAbits.RA2 = 1;  // disable AUDIO
        PORTBbits.RB5 = 0;  // XLAT_LED = 0    
        PORTBbits.RB3 = 0;  // enable MEM
        break;
    case SEL_AUDIO:
        PORTBbits.RB3 = 1;  // disable MEM 
        PORTBbits.RB5 = 0;  // XLAT_LED = 0            
        PORTAbits.RA2 = 0;  // enable AUDIO
        break;
    }
}


void initSPI()
{
    TRISCbits.TRISC5 = 0;   // SDO as output
    TRISCbits.TRISC4 = 1;   // SDI as input
    TRISCbits.TRISC3 = 0;   // SCK as output

    TRISBbits.TRISB3 = 0;   // flash mem /CS as output
    TRISBbits.TRISB5 = 0;   // XLAT_LED as output
    TRISAbits.TRISA2 = 0;   // audio /CS as output

    selectSPISlave(SEL_NONE);

    SSP1STATbits.SMP   = 0;     // sample in middle of data wave
    SSP1STATbits.CKE   = 1;     // SPI mode    
    SSP1CON1bits.CKP   = 0;     // clock polarity
    SSP1CON1bits.SSPM  = 0b0010;    // slowest SPI mode FOSC/64 = 375kHz
    SSP1CON1bits.SSPEN = 1;     // enable SPI
}


unsigned char spiInOut(unsigned char test)
{
    SSP1BUF = test;
    while(SSP1STATbits.BF == 0) {};
    return SSP1BUF;
}

// ************************************************************
// LEDS
// ************************************************************

void initLEDS()
{
    TRISBbits.TRISB5 = 0;   // LED XLAT output
    TRISAbits.TRISA5 = 0;   // LED MODE output
}

void enableLEDS(unsigned char enabled)
{
    selectSPISlave(SEL_NONE);   // make sure XLAT is 0
    PORTAbits.RA5 = 0;          // on/off register select

    // send 16 bits
    if (enabled)
    {
        spiInOut(0xFF);
        spiInOut(0xFF);
    }
    else
    {
        spiInOut(0xFF);
        spiInOut(0xFF);        
    }

    // toggle XLAT
    PORTBbits.RB5 = 1;
    printf("*");
    PORTBbits.RB5 = 0;
}


void setLEDBrightness()
{
    selectSPISlave(SEL_NONE);   // make sure XLAT is 0
    PORTAbits.RA5 = 1;          // brightness register select

    // send 112 (16*7) bits
    for(unsigned char i=0; i<14; i++)
    {
        spiInOut(0xAA);
    }
    
    // toggle XLAT
    PORTBbits.RB5 = 1;
    printf("*");
    PORTBbits.RB5 = 0;  
}

// ************************************************************
// MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN 
// ************************************************************

void main() 
{
    initUART1();
    initSPI();
    enableLEDS(1);
    setLEDBrightness();

    while(1)
    {
        printf("Hello, world!\n\r");
    };
}

