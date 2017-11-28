PICDIS18.PY -- a disassembler for PIC18 microcontroller code v0.3
Claudiu.Chiculita@ugal.ro    http://www.ac.ugal.ro/staff/ckiku/software/

picdis18.py  [-l] [-o outputfile] inputfile

inputfile  input .HEX file in Intel 8-bit (and extended) format
file_.asm  default output file, containing the assembly instructions, directives,
           SFR names, branch/call labels, callers of procedures, comments
can be loaded into MPLAB and reassembled immediatedly without any problems!
Although the processor type should be changed (default 18F252)


# The assigned labels contain the hex address of the instruction
# ToDo: + clean code + bitnames 
#------------------History---------------------------
#
#
#----------------------------------------------------

#The starting point for this program was:
#DISPIC16.PY
#Copyright (C) 2002 by Mel Wilson  mailto://mwilson@the-wire.com .
#Free for any use providing this notice is retained in the code.
