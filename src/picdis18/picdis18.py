"""PICDIS18.PY -- a disassembler for PIC18 microcontroller code v0.4
Claudiu.Chiculita@ugal.ro    http://www.ac.ugal.ro/staff/ckiku/software/

picdis18.py  [-h] [-l] [-o outputfile] file.hex

file.hex   input .HEX file in Intel format
file_.asm  default output file, containing the assembly instructions, SFR names
           directives, branch/call labels, callers of procedures, comments
-o	save result to the specified file
-h	0xHH syle for hex numbers (default is: HHh)
-l	lists addresses and binary code of instructions
Can be loaded into MPLAB and reassembled immediatedly without any problems!
although the processor type should be changed (default 18F252)
"""
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
#Use this code at your own risk.

import getopt, os, sys, string

debug = 0
tabsize = 4
listing = 0		# =1 => only 1 asm line per each addr. ( -empty +nil )
hexstyle = 0		# 0NNNh
code={}  # key=(even addresses),  value=Instruction(s)
eeprom={}; configuration={}
class Instruction:
	def __init__(self):
		self.bin=0			# taken from hex
		self.dummy=0		# strange jumps, ORGs
		self.calls=[]		# list of callers/(jumpers)
		self.asm=''
		self.label='\t'
		self.prefixline=''
		self.comment=';'

###################################################################
def hexc(nr):			#custom hex()
	if hexstyle:
		return '0x%X' % (nr,)		# C syle
	else:
		if nr<10: return str(nr)	# ASM style
		t = '%Xh' % (int(nr),)
		if t[0] in string.ascii_letters:
			t = '0' + t
		return t

###################################################################
def makelabel(nr):
	s = ('%X' % (int(nr),) )
	return 'p'+s.rjust(5).replace(' ','_')
	

###################################################################
# Reads an Intel HEX file into the `code` dictionary
# the keys are addresses and code[].bin has the object code 
#
def read_object_code (objectfile):
	global code
	#max_addr = len (code)		# top address loaded by the hex file
	exta = '0000'				# prefix for extended addr
	for x in objectfile.readlines():
		if x[0] == ':':			# Intel form		:CCAAAATTllhhllhh....rr
			x = x.strip()
			nb = int (x[1:3], 16)			# number of bytes this record
			ad = int (exta + x[3:7], 16)	# starting word address this record + extended linear
			ty = int (x[7:9], 16)			# record type
			if debug:	print(string.rjust(hexc(ad),8), ty, x)
			if   ty == 0:	pass
			elif ty == 1:	break			# end of record
			elif ty == 4:					# extended linear, only :02000004aaaa  supported
				exta = x[9:13];
				continue	
			else:
				print("\t\tNot a data record")
				continue 		# not a data record - ignore it
			teco = {}			# temporary code
			cks = nb + (ad & 0xFF) + ((ad >> 8) & 0xFF) + ty	# init checksum
			dd = x[9:-2]		# isolate the data
			ad /= 2				# convert byte to word address
			while dd:
				d = int (dd[0:2],16), int (dd[2:4],16)	# convert 2 bytes
				teco[ad*2] = Instruction()				# add a new key=address
				teco[ad*2].bin = (d[1] << 8) + d[0]		# write a word in code
				cks += d[0] + d[1]						# update checksum
				ad += 1									# bump the code address
				dd = dd[4:]								# drop old data
			if (int (x[-2:], 16) + cks) & 0xFF != 0:	# verify the checksum
				raise ValueError( (hexc (-cks), hexc (int(x[-2:],16))))
			if (ad < 0x300000/2):
				code.update(teco)
			elif (ad < 0xF00000/2):
				configuration.update(teco)
			else:
				eeprom.update(teco)
		else:
			print("Ignoring line: ", x)					# ignore anything else
# end read_object_code

def read_registry_names():
	# Provide symbolic names for all special file registers
	f = open ('regnames18.txt', "r");
	regn = {}
	rn = f.readlines()
	for x in rn:
		x = x.strip()
		if x == "":
			continue
		a, b = x.split (' ')
		regn[int(a,16)] = b
	f.close()
	return regn

###################################################################
# Read a specially-formatted string to build the opcode-identification table.
# Each line in the master string contains
# - an assembly statement template in lowercase with 'magic' uppercase characters
#   denoting operands to be filled in
# - a tab character
# - a binary instruction word template containing '1's and '0's in critical positions
#   and with any other non-blanks indicating "don't care" bit positions
# Each entry in the opcode-identification table contains:
# - an assembly template string
# - a binary value
# - a binary mask
# see the `matching_opcode` and `assembly_string` functions for the
# examples of the use of this table
#
def make_operand_table ():
	f = open ('opcodes18.txt', "r");
	oplist = []
	opcode_templates = f.readlines()
	for x in opcode_templates:#.split ('\n'):
		x = x.strip()
		if x == "":
			continue
		a, c = x.split ('\t')		# split into (asm_template, bit_template)
		cv = cm = 0					# init code_value, code_mask
		for ch in c:				# for each character in the bit template
			if ch == '0':
				cv = 0 | (cv << 1)	# set the bit value 0
				cm = 1 | (cm << 1)	# set mask to compare this bit
			elif ch == '1':
				cv = 1 | (cv << 1)	# set the bit value 1
				cm = 1 | (cm << 1)	# set mask to compare this bit
			elif ch == ' ':
				pass				# ignore blanks.. they're just for human readablilty
			else:
				cv = 0 | (cv << 1)	# value 0 to ignore this bit position
				cm = 0 | (cm << 1)	# mask  0 to ignore this bit position
		a = a.replace (' ', '\t')	# make the opcode/operands separator a tab
		oplist.append ((a, cv, cm))	# append a (asm_template, value, mask) tuple
	f.close()
	return tuple (oplist)			# Eg: ('addwf	F, D, A', 0x4800, 0xfc00)


###################################################################
# Return the assembly-language template string
# that matches a given binary instruction word
#
def matching_opcode (w):
	global operand_table
	for x in operand_table:
		if (w & x[2]) == x[1]:		# code[].bin & code_mask == code_value
			return x[0]
	return "X"						# unidentifiable binary -- punt

def lookup_adr(addr):
	global code
	if addr in code:
		return code[addr] 
	else:							# exceptional case: a jump to a location not defined in hexfile 
		x = Instruction()			#					or a missing second word in a dword instr
		x.dummy = 1
		x.bin = 0xffff
		code[addr] = x
		return x
	

###################################################################
# F		.... .... ffff ffff
# N		.... .... nnnn nnnn
# M		.... .nnn nnnn nnnn
# C		.... .... .... kkkk
# K		.... .... kkkk kkkk
# B		.... bbb. .... ....
# A		.... ...a .... ....
# D		.... ..d. .... ....
# S		.... .... .... ...s
# W		double  -call,goto
# Y		double  -movff
# Z		double  -lfsr
def assembly_line (addr):
	global code
	cod = code[addr]
	w = cod.bin
	t = matching_opcode (w)		# get the right assembly template
	af = w & 0x100				# `guess` `A` flag
	if debug: print(hexc(w), t)
	s = []						# init the return value
	for c in t:					# for each character in the template
		if   c == 'F':			# insert a register-file address
			q = w & 0xFF
			if af==0 and q>=0x80:
				s.append (reg_names.get (q | 0xF00, hexc(q)))
			else:
				s.append (hexc(q))
		elif c == 'D':			# insert a ",w" modifier=0, if appropriate
			if (w & 0x200) == 0:
				s.append ('W')
			else:
			    s.append ('f')
		elif c == 'B':			# insert a bit-number
			s.append ( '%d' % (((w >> 9) & 0x7),) )
		elif c == 'K':			# insert an 8-bit constant
			s.append (hexc(w & 0xFF))
		elif c == 'C':			# movlb
			s.append (hexc(w & 0xF))
		elif c == 'N':			# branch relative +- 127
			q = w & 0xFF
			if q < 0x80:
				dest = addr + 2 + q*2
			else:
				dest = addr + 2 - (0x100 - q)*2
			s.append (makelabel(dest))
			lookup_adr(dest).calls.append(addr)
		elif c == 'M':			# insert a rcall/bra relative +- 1023 
			q = w & 0x7FF
			if q < 0x400:
				dest = addr + 2 + q*2
			else:
				dest = addr + 2 - (0x800 - q)*2
			s.append (makelabel(dest))
			lookup_adr(dest).calls.append(addr)
		elif c == 'A':			# access bank = 0 implicit
			if (w & 0x100) != 0:
				s.append ('BANKED')
			elif s[-3:] == [',','f',',']:# do not show:	,f,0
				s = s[:-3]
			elif s[-1] == ',':	# do not show:	,0
				del s[-1]
		elif c == 'S':			# =1 restore reg. on ret: retfie/return (implicit 0)
			if (w & 0x1) == 1:
				s.append ('FAST')
			elif s[-1] == ',':
				del s[-1]
		elif c == 'Y':			# dword	movff
			w2 = lookup_adr(addr+2).bin
			s.append (reg_names.get (w  & 0xFFF, hexc(w  & 0xFFF)) +','+
					  reg_names.get (w2 & 0xFFF, hexc(w2 & 0xFFF)) )
		elif c == 'W':			# dword	call/goto
			w2 = lookup_adr(addr+2).bin
			dest = ((w & 0xFF) | ((w2 & 0xFFF) << 8))*2
			lookup_adr(dest).calls.append(addr)
			s.append (makelabel(dest))
			if ((w & 0x300) ^ 0x100) == 0:	# only if its a 'call' and 's' is set
				s.append (',FAST')
		elif c == 'Z':			# dword	lfsr
			w2 = lookup_adr(addr+2).bin
			s.append (str((w & 0x30) >> 4) +','+ hexc(((w & 0xF) << 8) | (w2 & 0xFF)) )
		elif c == 'X':			# insert the hex version of the whole word
			s.append ('DE ' + hexc(w) + '\t\t;WARNING: unknown instruction!')
		else:					# insert this source-code character
			if (c=='\t')and(len(s)<tabsize):
				s.append('\t')	# for short instructions (eg: bfc/bz/...) add another tab
			s.append (c);
	code[addr].asm = ''.join (s)
# end assembly_string

def eep_cfg_txt():					# generate text for the eeprom and configuration words
	txt = ''
	for x in configuration:
		if listing:
			txt += '%06X %04X\n' % (int(x), int(configuration[x].bin))
		else:
			txt += '\t\t__CONFIG %s, %s\n' % (hexc(x), hexc(configuration[x].bin))
	if eeprom:
		txt += '\t\t;eeprom:\n'
		for x in eeprom:			# hopefully the hex will have very fey eeprom location defined
			if listing:
				txt += '%06X %04X\n' % (int(x), int(eeprom[x].bin))
			else:
				txt += '\t\tORG %s\n\t\tDE %s\n' % (hexc(x), hexc(eeprom[x].bin))
	if txt:
		txt += '\n\n'
	return txt

###############################################################
# main entry to the program
###############################################################
if __name__ == '__main__':
	try:
		opts, args = getopt.getopt (sys.argv[1:], "hlo:")
		input_file = args[0]
		output_file = input_file[:-4] + '_.asm'
		for o, v in opts:
			if o == '-o':	# user-supplied output path
				output_file = v
			elif o == '-l':
				listing = 1
			elif o == '-h':	#ToDo:repair
				hexstyle = 1	# 0xNNN
	except:
		print(__doc__)
		sys.exit(2)
	
	print('Building tables...')
	operand_table = make_operand_table ()
	reg_names = read_registry_names()

	print('Reading object file...', os.path.abspath (input_file))
	read_object_code (open (input_file, "r"))

	print('Disassemble...')
	tempk = list(code.keys())
	tempk.sort()
	for wadr in tempk:
		assembly_line (wadr)

	print('Arranging...')						# (old tempk)!
	for wadr in tempk:
		cod = code[wadr]
		if len(cod.calls)>0:					# put labels
			cod.label = makelabel(wadr)
			cod.comment += ' entry from: ' + ','.join(map(hexc,cod.calls))
			if (not listing) and (len(cod.calls) > 1):
				cod.prefixline += '\n'
		if len(cod.comment) < 3:				#remove empty comments
			cod.comment = ''
		else:
			s = cod.asm.expandtabs(tabsize)		#align tabs
			cod.comment = '\t'* int((32+3-len(s))/tabsize) + cod.comment
		if wadr-2 not in code:			# must put an ORG if not contiguous
			cod.prefixline += '\t\tORG %s\n' % (hexc(wadr),)
	for wadr in tempk:		#could not mix with ORG
		if (not listing) and (code[wadr].asm.strip() == ';nil'):
			del code[wadr]						# remove ;nil-s

	print('Writing...', os.path.abspath (output_file))
	otf = open (output_file, "w")
	tempk = list(code.keys())
	tempk.sort()
	otf.write(';Generated by PICDIS18, Claudiu Chiculita, 2003.  http://www.ac.ugal.ro/staff/ckiku/software\n')
	otf.write('\t\t;Select your processor\n\t\tLIST      P=18F252\t\t; modify this\n\t\t#include "p18f252.inc"\t\t; and this\n\n')
	otf.write(eep_cfg_txt())
	for a in tempk:
		if listing:
			otf.write('%05X %04X\t' % (int(a), int(code[a].bin)))
		else:
			otf.write(code[a].prefixline)
		otf.write ('%s\t%s%s\n' % (code[a].label, code[a].asm, code[a].comment))
		#otf.write ('%04X: %04X%s\t%s\t\t\t%s\n' % (a, code[a].bin, code[a].label, code[a].asm, code[a].comment))
	otf.write('\tEND')
	otf.close()
	print('Done.')

