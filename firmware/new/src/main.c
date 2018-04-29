#define _XTAL_FREQ 24000000
#pragma config WDT = OFF
#pragma config XINST = OFF
#pragma config OSC  = HS
#pragma config PWRT = OFF


#include <xc.h>

void main() 
{
    //ADCON1 |= 0x0F;                 // Configure AN pins as digital
    //CMCON  |= 7;                    // Disable comparators

    while(1)
    {

    };
}

