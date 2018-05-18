
#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hardware.h"
#include "protocol.h"

//#define DEBUGPROTOCOL

unsigned char g_ledData[16];

void main() 
{
    unsigned char prevButtonState = 0;  // previous button state
    initHardware();

    memset(g_ledData, 0, sizeof(g_ledData));
    setLEDBrightness(g_ledData);

    protocolState_t state;
    state.state = S_IDLE;
    state.tok_state = 0;

    while(1)
    {
        // process bytes coming into the UART one by one
        if (UART1hasBytes())
        {
            unsigned char ret = 0;
            
            unsigned char c = UART1getByte();

            // write byte to the console for debugging
            UART1putByte(c);

            // tokenize the incoming UART byte stream and
            // issue commands when one is decoded.
            ret = protoSubmitToken(&state, protoSubmitByte(&state, c));
            if (ret == CMD_LED)
            {
                unsigned char ledIdx = state.args[0];
                if (ledIdx > 5) ledIdx = 5;
                ledIdx = ledIdx * 3;

                g_ledData[ledIdx]   = state.args[1]; // r
                g_ledData[ledIdx+1] = state.args[2]; // b
                g_ledData[ledIdx+2] = state.args[3]; // g
                setLEDBrightness(g_ledData);

                #ifdef DEBUGPROTOCOL
                printf("LED %d r=%d b=%d g=%d\n\r", state.args[0], state.args[1], state.args[2], state.args[3]);
                #endif

                UART1putByte('+');  // confirm command
            }
            else if (ret == CMD_MOTOR)
            {
                #ifdef DEBUGPROTOCOL
                printf("MOTOR %u dir=%u pulses=%u\n\r", state.args[0], state.args[1], state.args[2]);
                #endif

                runMotor(state.args[0], state.args[1], state.args[2]);

                UART1putByte('+');  // confirm command
            }
            else if (ret == CMD_UPDATE)
            {
                #ifdef DEBUGPROTOCOL
                printf("LED update\n\r");
                #endif

                UART1putByte('+');  // confirm command
            }
            else if (ret == CMD_REPORTMOTORS)
            {
                printf("\n\r");
                printf("MOTOR1 %u\n\r", getMotor1Count());
                printf("MOTOR2 %u\n\r", getMotor2Count());
                UART1putByte('+');  // confirm command
            }
            else if (ret == CMD_REPORTTIMER)
            {
                printf("\n\r");
                printf("TIMER %u\n\r", getTimer());
                UART1putByte('+');  // confirm command
            }
            else if (ret == CMD_HOMEMOTORS)
            {
                homeMotors();
                UART1putByte('+');  // confirm command
            } 
        }

        //FIXME: we need to test the de-bounce the button!
        unsigned char buttonRawState = readButton();
        unsigned char buttonState = debounceButton(buttonRawState);
        if ((prevButtonState != buttonState))
        {
            prevButtonState = buttonState;
            if (buttonState == 0)
            {
                UART1putByte('B');  // button pressed
            }
            else
            {
                UART1putByte('b');  // button released
            }
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

