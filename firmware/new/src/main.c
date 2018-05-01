#define _XTAL_FREQ 24000000
#pragma config WDT = OFF
#pragma config XINST = OFF
#pragma config OSC  = HS
#pragma config PWRT = OFF
#pragma config LVP = OFF

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

    SSP1STAT = 0;
    SSP1STATbits.SMP   = 0;     // sample in middle of data wave
    SSP1STATbits.CKE   = 1;     // SPI mode    
    SSP1CON1 = 0;
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
        spiInOut(0x00);
        spiInOut(0x00);
    }

    // toggle XLAT
    PORTBbits.RB5 = 1;
    PORTBbits.RB5 = 0;
}

/** Send 16x 7-bit words (112 bits) in a packed 14x8-bit burst. 
    The MSB of each ledData byte is ignored.

    ledData[0]  : top LED red
    ledData[1]  : top LED blue
    ledData[2]  : top LED green
    ledData[3]  : middle right LED red
    ledData[4]  : middle right LED blue
    ledData[5]  : middle right LED green
    ledData[6]  : middle LED red
    ledData[7]  : middle LED blue
    ledData[8]  : middle LED green
    ledData[9]  : middle left LED red
    ledData[10] : middle left LED blue
    ledData[11] : middle left LED green
    ledData[12] : bottom LED red
    ledData[13] : bottom LED blue
    ledData[14] : bottom LED green
    ledData[15] : none
*/
void setLEDBrightness(unsigned char *ledData, unsigned char testMode)
{
    selectSPISlave(SEL_NONE);   // make sure XLAT is 0
    PORTAbits.RA5 = 1;          // brightness register select


    if (testMode == 0)
    {
        // inefficient but workable code
        unsigned char  word = 0;
        unsigned char  shiftcnt = 6;    // index of bit to be extracted from ledData
        unsigned char  bitcnt = 0;      // number of bits shifted into 'word'
        unsigned short totalWords = 14; // number of 8-bits words to transmit
        while(totalWords > 0)
        {
            word <<= 1;
            word |= ((ledData[0]>>shiftcnt) & 0x01);    // set LSB of word
            if (shiftcnt == 0)
            {
                // next LED value
                shiftcnt = 6;
                ledData++;
            }
            else
            {
                // advance to next LED bit
                shiftcnt--;
            }

            // check if we're ready to send out
            // an 8-bit word to the SPI
            bitcnt++;
            if (bitcnt == 8)
            {
                bitcnt = 0;
                spiInOut(word);
                totalWords--;
            }
        }
    }
    else
    {
        // test mode
        for(unsigned char i=0; i<14; i++)
        {
            spiInOut(0xFF);
        }
    }    
    // toggle XLAT
    PORTBbits.RB5 = 1;
    PORTBbits.RB5 = 0;  
}

// ************************************************************
// Button
// ************************************************************

void initButton()
{
    TRISBbits.TRISB0 = 1;   // Switch as input
}

unsigned char readButton()
{
    return PORTBbits.RB0;
}

// ************************************************************
// Motor control
// ************************************************************

typedef enum 
{
    MOTOR_STOP = 0,
    MOTOR_CLOCKWISE,
    MOTOR_ANTICLOCKWISE
} motor_t;


void setMotor1(motor_t state)
{
    // 0,1 is valid
    // 0,0 is valid
    // 1,0 is valid
    // 1,1 does not appear in the source code
    switch(state)
    {
    default:
    case MOTOR_STOP:
        PORTFbits.RF0 = 0;
        PORTFbits.RF1 = 0;
        break;
    case MOTOR_CLOCKWISE:
        PORTFbits.RF0 = 1;
        PORTFbits.RF1 = 0;
        break;
    case MOTOR_ANTICLOCKWISE:
        PORTFbits.RF0 = 0;
        PORTFbits.RF1 = 1;
        break;        
    }
}

void setMotor2(motor_t state)
{
    switch(state)
    {
    default:
    case MOTOR_STOP:
        PORTFbits.RF2 = 0;
        PORTFbits.RF3 = 0;
        break;
    case MOTOR_CLOCKWISE:
        PORTFbits.RF2 = 1;
        PORTFbits.RF3 = 0;
        break;
    case MOTOR_ANTICLOCKWISE:
        PORTFbits.RF2 = 0;
        PORTFbits.RF3 = 1;
        break;        
    }
}

void initMotors()
{
    setMotor1(MOTOR_STOP);
    setMotor2(MOTOR_STOP);
    TRISFbits.TRISF0 = 0;   // MCC1A output
    TRISFbits.TRISF1 = 0;   // MCC1B output
    TRISFbits.TRISF2 = 0;   // MCC2A output
    TRISFbits.TRISF3 = 0;   // MCC2B output
    setMotor1(MOTOR_STOP);
    setMotor2(MOTOR_STOP);    
}


// ************************************************************
// Init Audio
// ************************************************************

const unsigned char OKI_INIT_TBL[19*2] =
{
    0x11, 0x63,
    0x13, 0x21,
    0x15, 0x00,
    0x19, 0x01,
    0x17, 0x1D,
    0x17, 0x00,
    0x57, 0x08,
    0x55, 0x00,
    0x41, 0xFF,
    0x43, 0x00,
    0x45, 0xFF,
    0x63, 0x00,
    0x37, 0x0F,
    0x39, 0x00,
    0x67, 0x19,
    0x25, 0xCB,
    0x21, 0x40,
    0x23, 0xC0,
    0x35, 0x01
};

void initAudio()
{
    // setup clock to the OKI chip
    // using ECCP1
    TRISCbits.TRISC2 = 0;       // ECCP1 / P1A as output to OKI CLK
    TRISBbits.TRISB2 = 1;       // OKI IRQ input
    TRISAbits.TRISA1 = 0;       // OKI /RST

    // Reset the OKI chip
    PORTAbits.RA1 = 0;

    // Measured OKI clock on original Nabaztag: 
    // 6 MHz 50% duty cycle:

    CCP1CON = 0x2C;         // PWM mode
    CCPR1L = 0;
    T2CON = 0;              // timer 2 off, no prescalers
    PR2 = 0;
    T2CONbits.TMR2ON = 1;   // timer 2 on
    
}

void resetAudio()
{
    PORTAbits.RA1 = 0;  // pull /OKI_RST low
    PORTAbits.RA1 = 1;
}

unsigned char ioAudioReg(unsigned char regNo, unsigned char data)
{
    selectSPISlave(SEL_AUDIO);
    spiInOut(regNo);
    unsigned char readData = spiInOut(data);
    selectSPISlave(SEL_NONE);

    return readData;
}

void setupOKIRegs()
{
    printf("Setting up OKI registers..\n\r");
    unsigned char idx = 0;
    for(unsigned char i=0; i<19; i++)
    {
        selectSPISlave(SEL_AUDIO);
        ioAudioReg(OKI_INIT_TBL[idx], OKI_INIT_TBL[idx+1]);
        idx += 2;
        printf("+");    // delay
        selectSPISlave(SEL_NONE);
    }
    printf("\n\r");
}

// ************************************************************
// MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN 
// ************************************************************

void main() 
{
    unsigned char ledData[16];

    ADCON0 = 1;             // original Nabaztag
    ADCON1 = 0b00001110;    // original Nabaztag : value 0x0E
    ADCON2 = 6;             // original Nabaztag
    CMCON  = 7;             // original Nabaztag
    CVRCON = 0;             // original Nabaztag

    initUART1();
    initSPI();
    initButton();
    initMotors();
    initLEDS();
    initAudio();
    enableLEDS(1);
    resetAudio();
    setupOKIRegs();

    //ioAudioReg(0x67, 0x11); // take OKI out of power down
    //ioAudioReg(0x63, 0x04); // 50% volume ( one channel only)    
    //ioAudioReg(0x43, 0xFF); // portio -> all outputs
    //ioAudioReg(0x41, 0x00); // port mode -> control by external CPU
    //ioAudioReg(0x45, 0x01); // enable amplifier

    unsigned char led_idx = 0;
    unsigned char buttonState = 0;
    while(1)
    {
        //printf("Hello, world!\n\r");
        for(unsigned char i=0; i<16; i++)
        {
            ledData[i] = 0;
        }
        ledData[led_idx] = 0x7F;
        setLEDBrightness(ledData, 0);

        unsigned char newState = readButton();
        if (newState != buttonState)
        {
            if (newState == 0)  // button pressed
            {
                led_idx++;
                led_idx &= 0x0F;
                printf("Led IDX = %d\n\r", led_idx);
            }
            buttonState = newState;            
        }

        if (newState == 0)
        {
            // button pressed
            setMotor1(MOTOR_CLOCKWISE);
            setMotor2(MOTOR_CLOCKWISE);
        }
        else
        {
            setMotor1(MOTOR_STOP);
            setMotor2(MOTOR_STOP);
        }
    }
    
}

