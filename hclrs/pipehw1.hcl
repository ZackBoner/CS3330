########## the PC and condition codes registers #############
register fF { 
	pc:64 = 0; 
}

register fD {
	stat:3 = STAT_AOK;
	icode:4 = NOP;
	ifun:4 = ALWAYS;
	rA:4 = 0;
	rB:4 = 0;
	valC:64 = 0;
	valP:64 = 0;	
}

register dE {
	stat:3 = STAT_AOK;
	icode:4 = NOP;
	ifun:4 = ALWAYS;
	dstE:4 = REG_NONE;
	#dstM:4 = REG_NONE;
	valA:64 = 0;
	valB:64 = 0;
	valC:64 = 0;
	srcA:4 = REG_NONE;
	srcB:4 = REG_NONE;
}

register eM {
	stat:3 = STAT_AOK;
	icode:4 = NOP;
	dstE:4 = REG_NONE;
	#dstM:4 = REG_NONE;
	valE:64 = 0;
	Cnd:1 = 1;
}

register mW {
	stat:3 = STAT_AOK;
	icode:4 = NOP;
	valE:64 = 0;
	dstE:4 = REG_NONE;
}

register cC {
	SF:1 = 0;
	ZF:1 = 1;
}

########## Fetch #############
pc = F_pc;

f_icode = i10bytes[4..8];
f_ifun = i10bytes[0..4];
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

########## Decode #############

# source selection
reg_srcA = [
        D_icode in {RRMOVQ, OPQ} : D_rA;
        1 : REG_NONE;
];
reg_srcB = [
        D_icode in {IRMOVQ, RRMOVQ, OPQ} : D_rB;
        1 : REG_NONE;
];

d_srcA = reg_srcA;
d_srcB = reg_srcB;

d_valA = [ # directly from book
	D_icode in { CALL, JXX } : D_valP; # Use incremented PC
	reg_srcA == e_dstE : e_valE;	# Forward valE from execute
	#reg_srcA == M_dstM : m_valM;	# Forward valM from memory
	reg_srcA == M_dstE : M_valE;	# Forward valE from memory
	#reg_srcA == W_dstM : W_valM;	# Forward valM from write back
	reg_srcA == W_dstE : W_valE;	# Forward valE from write back
	1 : reg_outputA; # Use value read from register file
];

d_valB = [
	reg_srcB == e_dstE : e_valE;      # Forward valE from execute
        #reg_srcB == M_dstM : m_valM;      # Forward valM from memory
        reg_srcB == M_dstE : M_valE;      # Forward valE from memory
        #reg_srcB == W_dstM : W_valM;      # Forward valM from write back
        reg_srcB == W_dstE : W_valE;      # Forward valE from write back
        1 : reg_outputB; # Use value read from register file
];

d_valC = D_valC;

d_dstE = [
	D_icode in {IRMOVQ, RRMOVQ, OPQ} : D_rB;
	1 : REG_NONE;
];
#d_dstM = reg_dstM;

d_stat = D_stat;
d_icode = D_icode;
d_ifun = D_ifun;

########## Execute #############

e_stat = E_stat;
e_icode = E_icode;

wire aluA:64;
aluA = [
	E_icode in {RRMOVQ, OPQ} : E_valA;
	E_icode in {IRMOVQ} : E_valC;
	1 : 0;
];

wire aluB:64;
aluB = [
	E_icode in {OPQ}: E_valB;
	E_icode in {RRMOVQ, IRMOVQ} : 0;
	1 : 0;
];

wire alufun:4;
alufun = [
	E_icode == OPQ : E_ifun;
	1 : ADDQ;
];

e_valE = [
	alufun == ADDQ : aluB + aluA;
	alufun == SUBQ : aluB - aluA;
	alufun == ANDQ : aluB & aluA;
	alufun == XORQ : aluB ^ aluA;
	1 : 0; # won't ever happen
];

e_dstE = [
	E_icode == CMOVXX && !e_Cnd : REG_NONE;
	1 : E_dstE;
];

#e_dstM = E_dstM;

stall_C = (E_icode != OPQ) || !(m_stat == STAT_AOK) || !(W_stat == STAT_AOK);
c_ZF = (e_valE == 0);
c_SF = (e_valE >= 0x8000000000000000);

e_Cnd = [
        E_ifun == ALWAYS : 1; # implements RRMOVQ
        E_ifun == LE : C_ZF || C_SF;
        E_ifun == LT : C_SF;
        E_ifun == EQ : C_ZF;
        E_ifun == NE : !C_ZF;
        E_ifun == GE : C_ZF || !C_SF;
        E_ifun == GT : !C_SF && !C_ZF;
        1          : 0;
];

########## Memory #############

m_stat = M_stat;
m_icode = M_icode;

m_valE = M_valE;
m_dstE = M_dstE;

########## Writeback #############

# destination selection
reg_dstE = [
        W_icode in {IRMOVQ, RRMOVQ, OPQ} : W_dstE;
        1 : REG_NONE;
];

reg_inputE = [ # unlike book, we handle the "forwarding" actions (something + 0) here
	W_icode in {RRMOVQ, IRMOVQ, OPQ} : W_valE;
        1: 0xBADBADBAD;
];

########## PC and Status updates #############

Stat = W_stat;

f_pc = [
	f_stat == STAT_AOK : f_valP;
	1 : F_pc;
];


