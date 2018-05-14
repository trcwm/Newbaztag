
#include <xc.h>
#include <stdio.h>
#include "hardware.h"

void main() 
{
    initHardware();

    while(1)
    {
        if (UART1hasBytes())
        {
            UART1putByte(UART1getByte());
        }
    }

#if 0
    unsigned char led_idx = 0;
    unsigned char buttonState = 0;
    while(1)
    {
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

        if (UART1hasBytes())
        {
            UART1putByte(UART1getByte());
        }
    }
    #endif
}

