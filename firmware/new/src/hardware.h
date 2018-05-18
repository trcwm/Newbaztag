// ************************************************************
// ** HARDWARE 
// ************************************************************

#ifndef hardware_h
#define hardware_h

#include <stdint.h>

typedef enum 
{
    MOTOR_STOP = 0,
    MOTOR_CLOCKWISE,
    MOTOR_ANTICLOCKWISE
} motor_t;


void initHardware();

void UART1putByte(unsigned char c);
unsigned char UART1getByte();
unsigned char UART1hasBytes();

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
void setLEDBrightness(unsigned char *ledData);

unsigned char readButton();

/* Init/Reset the position of the ear motors */
void homeMotors();
void runMotor(uint8_t ID, uint8_t direction, uint8_t pulses);

uint16_t getMotor1Count();
uint16_t getMotor2Count();

/** read high-speed timer (runs at 750kHz) */
void     resetTimer();
uint16_t getTimer();

unsigned char debounceButton(unsigned char buttonRaw) ;

#endif
