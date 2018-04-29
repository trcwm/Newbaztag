Information provided by Niels and Gerrit.

### System view
The LED driver, OKI audio chip and external flash memory are all connected to the same SPI bus.

### Components
* Microchip PIC18F6627 I/PT main processor with 24 MHz crystal.
* TI L293DNE motor driver.
* AT45DB161 flash memory.
* TLC5922 16 channel LED driver.
* MPC6002 opamp.
* ML2870A audio codec.

### PIC18F6627 pin assignments

Speculative ICSP pins:

* RG5: MCLR/Vpp pin.
* RB5: PGM pin.
* RB6: PGC serial clock pin.
* RB7: PGD serial data pin.

MSSP is probably used to drive the external serial flash.

### Nabaztag 5-pin header J2

Compatible with PICKIT 3 ICSP header

* pin 1 -> serial clock (RB6)
* pin 2 -> serial data (RB7)
* pin 3 -> GND 
* pin 4 -> VDD 3.3V
* pin 5 -> MCLR/Vpp

Note: this looks like it is compatible with the PicKit3 programmer.

### Nabaztag 4-pin serial header J3

* pin 1 -> TXD
* pin 2 -> RXD
* pin 3 -> GND
* pin 4 -> VDD 3.3V

### EAR motors

EAR1:
* RF0 - MCC1A - output to L293DNE port 1A
* RD1 - MCC1B - output to L293DNE port 2A
Gerrit remarks: RA3 to REF MCC1 is cut through

EAR2:
* RF2 - MCC2A - output to L293DNE port 3A
* RF3 - MCC2B - output to L293DNE port 4A
Gerrit remarks: RC1 to REF MCC2 is cut through

Ear position comparator (MCP6002)
* RA4 - CMPT MCC1 - input from MCP6002 port OUT A
* RC0 - CMPT MCC2 - input from MCP6002 port OUT B

### RGB LEDs
LED driver is TLC5922 in SPI mode
* RA5 - TLC5922 MODE pin
* RB5 - TLC5922 XLAT pin
* RC3 - TLC5922 SCK pin
* RC5 - TLC5922 SIN pin
Gerrit remarks: SOUT of TLC5922 not connected

### Head button
* RB0 - SWITCH

### 3-way volume sensor
* RA0 - SENSOR A/D (?)
3.3V direct, 3.3V through 1kohms, GND through 1kohms.

### Serial port
* RC6 - TX1
* RC7 - RX1

### Flash memory (AT45DB161B)
* RB1 - to RDY/BUSY(low)
* RB3 - to RST
* RB4 - to /CS
* RC3 - to SPI SCK
* RC4 - to SO
* RC5 - to SI

### Audio chip (ML2870A)
* RB2 - to IRQ
* RC2 - to CLK
* RA1 - to RST
* RA2 - to /CS
* RC3 - to SPI SCK
* RC4 - to SDOUT
* RC5 - to SDIN

### Processor configuration bits
FOSC   = 0x02.
WDTEN  = 0.
ADCON1 = 0Eh -> AN0 is used as A/D input.
CMCON  = 7

CCP1CON = 0x0C -> PWM mode (uses timer2)
T2CON   = 2 -> prescaler is 16

SPBRG1  = 0x9B (8-bit baud generator, 9600 baud)
BAUDCON = 01-0 0-00 (reset value)
SPBRGH1 = 0x00 (reset value)
TXSTA1:  BRGH=1 (addr 0x2004)
RCSTA1:  SPEN=1, CREN=1 (add 0x2008)

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