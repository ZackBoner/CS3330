########## the PC and condition codes registers #############
# fetch register
register fF {
	predPC:64 = 0; 
}

# decode register
register dD {
	stat:3 = 0;
	icode:4 = 0;
	ifun:4 = 0;
	rA:4 = 0;
	rB:4 = 0;
	valC:64 = 0;
	valP:64 = 0;
}

# execute register
register eE {
	stat:3 = 0;
	icode:4 = 0;
	ifun:4 = 0;
	valC:64 = 0;
	valA:64 = 0;
	valB:64 = 0;
	dstE:4 = 0;
	dstM:4 = 0;
	srcA:4 = 0;
	srcB:4 = 0;
}

# memory register
register mM {
	stat:3 = 0;
	icode:4 = 0;
	Cnd:1 = 0;
	valE:64 = 0;
	valA:64 = 0;
	dstE:4 = 0;
	dstM:4 = 0;
}

# write-back register
register wW {
	stat:3 = 0;
	icode:4 = 0;
	valE:64 = 0;
	valM:64 = 0;
	dstE:4 = 0;
	dstM:4 = 0;
}

# condition code register
register cC {
	SF:1 = 0;
	ZF:1 = 1;
}

########## Fetch #############
pc = F_predPC;

wire fetch_icode:4;
fetch_icode = i10bytes[4..8]; # TODO: Consider instruction memory error

wire fetch_stat:3;
fetch_stat = [
	fetch_icode == HALT : STAT_HLT;
	fetch_icode > 0xb : STAT_INS;
	1 : STAT_AOK;
];

d_stat = fetch_stat;
d_icode = fetch_icode;
d_ifun = i10bytes[0..4];
d_rA = i10bytes[12..16];
d_rB = i10bytes[8..12];

wire fetch_valC:64;
fetch_valC = [
	D_icode in { JXX } : i10bytes[8..72];
	1 : i10bytes[16..80];
];
d_valC = fetch_valC;

wire offset:64;
offset = [
	fetch_icode in { HALT, NOP, RET } : 1;
	fetch_icode in { RRMOVQ, OPQ, PUSHQ, POPQ } : 2;
	fetch_icode in { JXX, CALL } : 9;
	1 : 10;
];

# select PC
wire fetch_pc:64;
fetch_pc = [
	M_icode == JXX && !M_Cnd : M_valA;
	W_icode == RET : W_valM;
	1: F_predPC;
];
wire fetch_valP:64;
fetch_valP = fetch_pc + offset;
d_valP = fetch_valP;

f_predPC = [
	fetch_icode == HALT : F_predPC;
	fetch_icode in {JXX, CALL} : fetch_valC;
	1 : fetch_valP;
];

########## Decode #############

# source selection
wire decode_srcA: 4;
decode_srcA = [
	D_icode in {RRMOVQ, RMMOVQ, OPQ, PUSHQ}: D_rA;
	D_icode in {POPQ, RET}: REG_RSP;
	1 : REG_NONE;
];

wire decode_srcB: 4;
decode_srcB = [
	D_icode in {RRMOVQ, RMMOVQ, IRMOVQ, MRMOVQ, OPQ} : D_rB;
	D_icode in {PUSHQ, POPQ, CALL, RET}: REG_RSP;
	1 : REG_NONE;
];

e_srcA = decode_srcA;
e_srcB = decode_srcB;

e_dstE = [
	# TODO: implement CMOVXX
	D_icode in {RRMOVQ, IRMOVQ, OPQ} : D_rB;
	D_icode in {PUSHQ, POPQ, CALL, RET} : REG_RSP;
	1 : REG_NONE;
];

e_dstM = [
	D_icode in {MRMOVQ, POPQ} : D_rA;
	1 : REG_NONE;
];

e_valC = D_valC;
e_stat = D_stat;
e_icode = D_icode;
e_ifun = D_ifun;

# handle the register file
reg_srcA = decode_srcA;
reg_srcB = decode_srcB;
reg_inputE = W_valE;
reg_dstE = W_dstE;
reg_inputM = W_valM;
reg_dstM = W_dstM;

e_valA = [
	D_icode in {CALL, JXX} : D_valP;
	decode_srcA == execute_dstE : execute_valE;
	decode_srcA == M_dstM : memory_valM;
	decode_srcA == M_dstE : M_valE;
	decode_srcA == W_dstM : W_valM;
	decode_srcA == W_dstE : W_valE;
	1 : reg_outputA;
];

e_valB = [
	1 : reg_outputB;
];
# TODO: e_valB

########## Execute #############
wire alufun:4;
alufun = [
	E_icode == OPQ : E_ifun;
	1 : ADDQ;
];

wire aluA: 64;
aluA = [
	E_icode in {RRMOVQ, OPQ} : E_valA; # add CMOVXX eventually
	E_icode in {IRMOVQ, RRMOVQ, MRMOVQ} : E_valC;
	E_icode in {PUSHQ, CALL} : -8;
	E_icode in {POPQ, RET} : 8;
	1 : 0;
];

wire aluB: 64;
aluB = [
	E_icode in {OPQ, RMMOVQ, MRMOVQ, PUSHQ, POPQ, CALL, RET} : E_valB;
	1 : 0;
];

wire execute_valE:64;
execute_valE = [
	alufun == ADDQ : aluB + aluA;
	alufun == SUBQ : aluB - aluA;
	alufun == ANDQ : aluB & aluA;
	alufun == XORQ : aluB ^ aluA;
	1 : 0;
];

wire execute_dstE:4;
execute_dstE = E_dstE; # TODO: implement logic here

# stall_C = 0; TODO: deal with stall_C with W_stat and memory_stat
c_ZF = (execute_valE == 0);
c_SF = (execute_valE >= 0x8000000000000000);

wire execute_Cnd: 1;
execute_Cnd = [
        E_ifun == ALWAYS : 1; # implements RRMOVQ
        E_ifun == LE : C_ZF || C_SF;
        E_ifun == LT : C_SF;
        E_ifun == EQ : C_ZF;
        E_ifun == NE : !C_ZF;
        E_ifun == GE : C_ZF || !C_SF;
        E_ifun == GT : !C_SF && !C_ZF;
        1          : 0;
];

m_stat = E_stat;
m_icode = E_icode;
m_Cnd = execute_Cnd;
m_valE = execute_valE;
m_valA = E_valA;
# TODO : m_dstE
m_dstE = execute_dstE;
m_dstM = E_dstM;

########## Memory #############

mem_addr = [
        M_icode in {RMMOVQ, MRMOVQ, PUSHQ, CALL} : M_valE;
        M_icode in {POPQ, RET} : M_valA;
        1 : 0;
];
mem_readbit = [
        M_icode in {POPQ, MRMOVQ, RET} : 1;
        1 : 0;
];
mem_writebit = [
        M_icode in {CALL, PUSHQ, RMMOVQ} : 1;
        1 : 0;
];
mem_input = M_valA;

wire memory_valM:64;
memory_valM = mem_output;

w_stat = M_stat; # TODO: consider memory error
w_icode = M_icode;
w_valE = M_valE;
w_valM = memory_valM;
w_dstE = M_dstE;
w_dstM = M_dstM;

########## Writeback #############

# destination selection
#reg_dstE = [
#	icode in {IRMOVQ, RRMOVQ} : rB;
#	1 : REG_NONE;
#];
#
#reg_inputE = [ # unlike book, we handle the "forwarding" actions (something + 0) here
#	icode == RRMOVQ : reg_outputA;
#	icode in {IRMOVQ} : valC;
#        1: 0xBADBADBAD;
#];


########## Status update #############

Stat = [
	W_stat == STAT_BUB : STAT_AOK;
	1 : W_stat;
];



