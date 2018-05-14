You need the Microchip XC8 compiler to compile this project.

# Newbaztag control protocol description

Each command is transmitted in human readable ASCII at 9600 baud over the serial link.
Each command is terminated by ASCII CR (0x13), ASCII LF (0x010) or ASCII NULL (0x00).
The response to accepted commands is a '+'. The response to malformed commands is a '-'.
A 'B' is sent whenever the button on the Nabaztag is pressed.
All values are given in decimal.

* L n r g b : set LED n to R,G,B value.
* M n pos   : set Motor n to position 'pos'.

