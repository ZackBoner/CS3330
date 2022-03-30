# -*-sh-*- # this line enables partial syntax highlighting in emacs

######### The PC #############
register fF { pc:64 = 0; }


########## Fetch #############
pc = F_pc;

f_icode = i10bytes[4..8];
f_rA = i10bytes[12..16];
f_rB = i10bytes[8..12];

f_valC = [
	f_icode in { JXX } : i10bytes[8..72];
	1 : i10bytes[16..80];
];

wire offset:64;
offset = [
	f_icode in { HALT, NOP, RET } : 1;
	f_icode in { RRMOVQ, OPQ, PUSHQ, POPQ } : 2;
	f_icode in { JXX, CALL } : 9;
	1 : 10;
];
f_valP = F_pc + offset;

f_stat = [
        f_icode == HALT : STAT_HLT;
        f_icode > 0xb : STAT_INS;
        1 : STAT_AOK;
];

wire loadUse:1;
loadUse = ((reg_srcA != REG_NONE && reg_srcA == E_dstM) || (reg_srcB != REG_NONE && reg_srcB == E_dstM));

stall_F = loadUse;
stall_D = loadUse;
bubble_E = loadUse;

########## Decode #############
register fD {
	stat:3 = STAT_AOK;
	icode:4 = NOP;
	rA:4 = REG_NONE;
	rB:4 = REG_NONE;
	valC:64 = 0;
	valP:64 = 0;
}

reg_srcA = [
	D_icode in {RMMOVQ} : D_rA;
	1 : REG_NONE;
];
reg_srcB = [
	D_icode in {RMMOVQ, MRMOVQ} : D_rB;
	1 : REG_NONE;
];

d_dstM = [
        D_icode in {MRMOVQ} : D_rA;
        1: REG_NONE;
];

d_valA = [
	reg_srcA == REG_NONE : 0;
	reg_srcA == m_dstM : m_valM;
	reg_srcA == W_dstM : W_valM;
	1 : reg_outputA;
];

d_valB = [
	reg_srcB == REG_NONE : 0;
	reg_srcB == m_dstM : m_valM;
	reg_srcB == W_dstM : W_valM;
	1 : reg_outputB;
];

d_icode = D_icode;
d_valC = D_valC;

d_stat = D_stat;

########## Execute #############
register dE {
	stat:3 = STAT_AOK;
	icode:4 = NOP;
	dstM:4 = REG_NONE;
	valA:64 = 0;
	valB:64 = 0;
	valC:64 = 0;
}


wire operand1:64, operand2:64;

operand1 = [
	E_icode in { MRMOVQ, RMMOVQ } : E_valC;
	1: 0;
];
operand2 = [
	E_icode in { MRMOVQ, RMMOVQ } : E_valB;
	1: 0;
];

e_valE = [
	E_icode in { MRMOVQ, RMMOVQ } : operand1 + operand2;
	1 : 0;
];

e_icode = E_icode;

e_stat = E_stat;

e_valA = E_valA;
e_dstM = E_dstM;

########## Memory #############
register eM {
	stat:3 = STAT_AOK;
	icode:4 = NOP;
	valA:64 = 0;
	valE:64 = 0;
	dstM:4 = REG_NONE;
}

mem_readbit = M_icode in { MRMOVQ };
mem_writebit = M_icode in { RMMOVQ };
mem_addr = [
	M_icode in { MRMOVQ, RMMOVQ } : M_valE;
        1: 0xBADBADBAD;
];
mem_input = [
	M_icode in { RMMOVQ } : M_valA;
        1: 0xBADBADBAD;
];
m_valM = mem_output;

m_dstM = M_dstM;
m_icode = M_icode;
m_stat = M_stat;

########## Writeback #############
register mW {
	stat:3 = STAT_AOK;
	icode:4 = NOP;
	dstM:4 = REG_NONE;
	valM:64 = 0;
}


reg_dstM = W_dstM;
reg_inputM = [
	W_icode in {MRMOVQ} : W_valM;
        1: 0xBADBADBAD;
];


Stat = W_stat;

f_pc = f_valP;



