# An example file in our custom HCL variant, with lots of comments

register pP {  

	pc:64 = 0; # 64-bits wide; 0 is its default value.

} 

pc = P_pc;

wire opcode:8, icode:4;

opcode = i10bytes[0..8];   # first byte read from instruction memory
icode = opcode[4..8];      # top nibble of that byte

const TOO_BIG = 0xC; # the first unused icode in Y86-64

# some named constants are built-in: the icodes, ifuns, STAT_??? and REG_???


# irmovq
wire rB:4, valC:64;
rB = i10bytes[8..12];
valC = [
	icode in {2, 3} : i10bytes[16..80];
        icode == 7      : i10bytes[8..72];
        1               : 0;
];
reg_dstE = [
        icode in {2, 3} : rB;
        1               : REG_NONE;
];


# rrmovq
wire rA:4;
rA = i10bytes[12..16];
reg_srcA = [
	icode == 2 : rA;
	1 	    : REG_NONE;
];
reg_inputE = [
        icode == 2 : reg_outputA;
        icode == 3 : valC;
        1          : 0;
];


# Stat is a built-in output; STAT_HLT means "stop", STAT_AOK means 
# "continue".  The following uses the mux syntax described in the 
# textbook
Stat = [
        icode == HALT      : STAT_HLT;
        icode in {8, 9}    : STAT_INS;
        icode >= TOO_BIG   : STAT_INS;
        1                  : STAT_AOK;
];

p_pc = [
        icode in {HALT, 1, 9}     : P_pc + 1;
        icode in {2, 6, 0xA, 0xB} : P_pc + 2;
        icode == 7                : valC;
        icode == 8                : P_pc + 9;
        icode in {3, 4, 5}        : P_pc + 10;
        1                         : P_pc;
];
