// ************************************************************
// ** HARDWARE 
// ************************************************************

#ifndef hardware_h
#define hardware_h

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

void setLEDBrightness(unsigned char *ledData);

unsigned char readButton();

void setMotor1(motor_t state);
void setMotor2(motor_t state);

#endif
