### Components
* Microchip PIC18F6627 I/PT main processor with 24 MHz crystal.
* TI L293DNE motor driver.
* AT45DB161 flash memory.
* TLC5922 16 channel LED driver.
* MPC6002 opamp (use unknown).
* ML2870A audio codec.

### PIC18F6627 pin assignments

Speculative ICSP pins:

* RG5: MCLR/Vpp pin.
* RB5: PGM pin.
* RB6: PGC serial clock pin.
* RB7: PGD serial data pin.

MSSP is probably used to drive the external serial flash.

### Nabaztag 5-pin header J2

* pin 1 -> serial clock
* pin 2 -> serial data
* pin 3 -> GND ?
* pin 4 -> VDD ?
* pin 5 -> MCLR/Vpp

Note: this looks like it is compatible with the PicKit3 programmer.

### Nabaztag 4-pin serial header J3

* pin 1 -> ?
* pim 2 -> ?
* pin 3 -> GND
* pin 4 -> VDD

### Processor configuration bits
FOSC   = 0x02.
WDTEN  = 0.
ADCON1 = 0Eh -> AN0 is used as A/D input.
CMCON  = 7

CCP1CON = 0x0C -> PWM mode
T2CON   = 2 -> prescaler is 16 (addr 0x)

### Port A
0: input
1: output (1)
2: output (1)
5: output (0)

### Port B
0: input
1: input
2: input
3: output (1)
4: output (1)
5: output (0)
6: input ?
7: input ?

### Port C
0: input ?
1: input ?
2: output
3: output
4: input
5: output
6: output
7: input

### Port D
* set to all inputs

### Port E
* set to all outputs

### Port F
0: output (0)
1: output (0)
2: output (0)
3: output (0)
4: output
5: input
6: output
7: output

### Port G
0: output (0)
1: input (0)
2: input (0)
3: output (0)

### Firmware notes
* I/O port setup is at address 0x068E4.
* CCP1/T2 setup via W register at 0x25E0, called with 0 -> sets period reg to zero.