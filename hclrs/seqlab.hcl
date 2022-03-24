register pP {  
        # program counter register
	pc:64 = 0; # 64-bits wide; 0 is its default value.
}

register cC {
	# condition code register
	SF:1 = 0;
	ZF:1 = 1;
}

pc = P_pc;

# FETCH
wire icode:4, ifun:4, rA:4, rB:4, VD:64, Dest:64, valC:64;

icode = i10bytes[4..8];
ifun = i10bytes[0..4];
rB = i10bytes[8..12];
rA = i10bytes[12..16];
VD = i10bytes[16..80];
Dest = i10bytes[8..72];

valC = [
	icode in {IRMOVQ, MRMOVQ, RMMOVQ} : VD;
        icode in {JXX, CALL}      : Dest;
        1               : 0;
];

# DECODE
reg_srcA = [
	icode in {RRMOVQ, RMMOVQ, MRMOVQ, OPQ} : rA;
	1 	    : REG_NONE;
];
reg_srcB = [
	icode in {RRMOVQ, IRMOVQ, RMMOVQ, MRMOVQ, OPQ} : rB;
        1 : REG_NONE;
];

# EXECUTE
wire ALU_input:64, valE: 64;

ALU_input = [
	icode in {OPQ, CMOVXX} : reg_outputA;
	icode == RMMOVQ : valC;
	1	     : 0;
];

valE = [
	icode == RMMOVQ || (icode == OPQ && ifun == ADDQ) : reg_outputB + ALU_input;
	icode == OPQ && ifun == SUBQ : reg_outputB - ALU_input;
	icode == OPQ && ifun == ANDQ : reg_outputB & ALU_input;
	icode == OPQ && ifun == XORQ : reg_outputB ^ ALU_input;
	1	     : 0;
];

stall_C = (icode != OPQ);
c_ZF = (valE == 0);
c_SF = (valE >= 0x8000000000000000);

# MEMORY
mem_addr = [
	icode == RMMOVQ : valE;
	icode == MRMOVQ : reg_outputB;
	1 : 0;
];
mem_readbit = [
	icode == RMMOVQ : 0;
	icode == MRMOVQ : 1;
	1 : 0;
];
mem_writebit = [
	icode == RMMOVQ : 1;
	icode == MRMOVQ : 0;
	1 : 0;
];
mem_input = [
	icode == RMMOVQ : reg_outputA;
	1 : 0;
];


# WRITE-BACK
wire conditionsMet:1;

conditionsMet = [
	ifun == ALWAYS : 1; # implements RRMOVQ
	ifun == LE : C_ZF || C_SF;
	ifun == LT : C_SF;
	ifun == EQ : C_ZF;
	ifun == NE : !C_ZF;
	ifun == GE : C_ZF || !C_SF;
	ifun == GT : !C_SF && !C_ZF;
	1	   : 0;
];

reg_dstE = [
	!conditionsMet && icode == CMOVXX : REG_NONE;
        icode in {CMOVXX, IRMOVQ, OPQ} : rB;
        1               : REG_NONE;
];

reg_inputE = [
        icode == CMOVXX : reg_outputA;
        icode == IRMOVQ : valC;
	icode == OPQ : valE;
        1          : 0;
];


# Stat is a built-in output; STAT_HLT means "stop", STAT_AOK means 
# "continue".  The following uses the mux syntax described in the 
# textbook
const TOO_BIG = 0xC;

Stat = [
        icode == HALT      : STAT_HLT;
        icode in {CALL, RET}    : STAT_INS;
        icode >= TOO_BIG       : STAT_INS;
        1                  : STAT_AOK;
];

p_pc = [
        icode in {HALT, NOP, RET}     : P_pc + 1;
        icode in {CMOVXX, OPQ, PUSHQ, POPQ} : P_pc + 2;
        icode == JXX                : valC;
        icode == CALL                : P_pc + 9;
        icode in {IRMOVQ, RMMOVQ, MRMOVQ}        : P_pc + 10;
        1                         : P_pc;
];
