/* --- riscv64 simulator created by Shuitx --- */ 


#include "iostream"
#include "fstream"
#include "string.h"
#include "string"
#include "Simulation.h"
#include "storage.h"
#include "cache.h"
#include "memory.h"
#include "def.h"

using namespace std;

extern void read_elf();
extern unsigned long long gp;
extern unsigned int madr;
extern unsigned int endPC;
extern unsigned int entry;
extern FILE *file;

CacheStruct_ L1lines[300000], L2lines[300000], L3lines[300000];
Memory m;
Cache l1, l2, l3;

long long inst_num = 0;
int exit_flag = 0;
int step_flag = 0;

int total_inst = 0;
int total_cycle = 0;
int data_risk = 0;
int wrongs = 0;

char Reg_name[32][6] = {
	"zr", "ra", "sp", "gp", 
	"tp", "t0", "t1", "t2",
	"s0", "s1", "a0", "a1",
	"a2", "a3", "a4", "a5",
	"a6", "a7", "s2", "s3",
	"s4", "s5", "s6", "s7",
	"s8", "s9", "s10", "s11",
	"t3", "t4", "t5", "t6"
};

void print_reg_info() {
    for(int i = 0; i < 32; ++i) {
        printf("%-3s 0x%016llx", Reg_name[i], reg[i]);
        if(i % 4 == 3) printf("\n");
        else printf("     ");
    }
}

int main()
{
	read_elf();
	entry = madr;       //main function address
	PC = entry;         //set initial PC
	reg[3] = gp;	    //global data pointer
	reg[2] = MAX / 2;   //stack pointer

	//Build cache and memory
	l1.SetLower(&l2);
	l2.SetLower(&l3);
	l3.SetLower(&m);

	StorageStats s;
	memset(&s, 0, sizeof(s));
	m.SetStats(s);
	l1.SetStats(s);
	l2.SetStats(s);
	l3.SetStats(s);

	StorageLatency ml;
	ml.bus_latency = 0;
	ml.hit_latency = 100;
	m.SetLatency(ml);

	StorageLatency l1l;
	l1l.bus_latency = 0;
	l1l.hit_latency = 1;
	l1.SetLatency(l1l);

	StorageLatency l2l;
	l2l.bus_latency = 0;
	l2l.hit_latency = 8;
	l2.SetLatency(l2l);

	StorageLatency l3l;
	l3l.bus_latency = 0;
	l3l.hit_latency = 20;
	l3.SetLatency(l3l);
	
	m.AssignMemory(memory);
	CacheConfig_ l1config(64, 8, 64, 1, 0);
	l1.SetConfig(l1config, L1lines);
	CacheConfig_ l2config(64, 8, 512, 1, 0);
	l2.SetConfig(l2config, L2lines);
	CacheConfig_ l3config(64, 8, 16384, 1, 0);
	l3.SetConfig(l3config, L3lines);
	
    printf("whether run by step[0/1]:\n");
	scanf("%d", &step_flag);
	setbuf(stdin, NULL);
	memset(&IF_ID, 0, sizeof(IF_ID));
	memset(&ID_EX, 0, sizeof(ID_EX));
	memset(&EX_MEM, 0, sizeof(EX_MEM));
	memset(&MEM_WB, 0, sizeof(MEM_WB));
	simulate();
	cout << "simulate over!" << endl;
	return 0;
}

void simulate()
{
    int end = endPC - 4;
    printf("entry: 0x%08x   ", entry);
	printf("endPC: 0x%08x   ", end);
	printf("gp: 0x%08llx\n", gp);
	char cmd[101]; //提供gdb功能
	int stop_PC = 0, halt = 1;
	while (MEM_WB.PC != end)
	{
	    if(step_flag == 1) {
			while(halt == 1) {
                printf("-");
				cin.getline(cmd, 100);
				if(strcmp(cmd, "s") == 0) {
					break;
				}
				if(strcmp(cmd, "c") == 0) {
					halt = 0;
                    break;
				}
				else if(strcmp(cmd, "r") == 0) {
                    print_reg_info();
				}
				else if(cmd[0] == 'm') {
			        int rqaddr = 0, rqlen = 0;
			        sscanf(cmd, "m %x %d", &rqaddr, &rqlen);
			        for(int bl = 0; bl < rqlen; ++bl) {
				        char rqmemout;
				        int hit, time;
				        l1.HandleRequest(rqaddr + bl, 1, 1, &rqmemout, hit, time);
				        printf("%02x ", rqmemout);
				        if(bl % 16 == 15 || bl == rqlen - 1) printf("\n");
			        }	
				}
				else if(cmd[0] == 'g') {
					 sscanf(cmd, "g %x", &stop_PC);
				}
                else if(strcmp(cmd, "q") == 0) {
                    exit_flag = 1;
                    break;
                }
			}
        }
        if (exit_flag == 1)
			break;

		IF();
		ID();
		EX();
		MEM();
		WB();
		IF_ID = IF_ID_old;
		ID_EX = ID_EX_old;
		EX_MEM  = EX_MEM_old;
		MEM_WB = MEM_WB_old;
		reg[0] = 0;
		total_cycle += 1;
		if(PC == stop_PC) halt = 1;
	}
	printf("inst: %d\n", total_inst);
	printf("cycle: %d\n", total_cycle);
	printf("data risk: %d\n", data_risk);
	printf("wrongs: %d\n", wrongs);
	while(exit_flag == 0) {
		printf("-");
		cin.getline(cmd, 100);
		if(strcmp(cmd, "r") == 0) {
			print_reg_info();
		}
		else if(cmd[0] == 'm') {
			int rqaddr = 0, len = 0;
			sscanf(cmd, "m %x %d", &rqaddr, &len);
			for(int bl = 0; bl < len; ++bl) {
				char rqmemout;
				int hit, time;
				l1.HandleRequest(rqaddr + bl, 1, 1, &rqmemout, hit, time);
				printf("%02x ", rqmemout);
				if(bl % 16 == 15 || bl == len - 1) printf("\n");
			}	
		}
		else if(strcmp(cmd, "q") == 0) {
			exit_flag = 1;
			break;
		}
	}
}

//Instruction Fecth
void IF()
{
	//write IF_ID_old
	IF_ID_old.inst = *(int*)(memory + PC);
	IF_ID_old.PC = PC;
	PC = PC + 4;
}

//Instruction Decoding
bool ID()
{
	unsigned int opcode = 0;
	unsigned int fuc3 = 0, fuc7 = 0;
	int rs1 = 0, rs2 = 0, rd = 0;
	unsigned int imm12 = 0;
	unsigned int imm20 = 0;
	//Read IF_ID
	unsigned int inst = IF_ID.inst;
	int EXTop = 0;
	unsigned int EXTsrc = 0;

	char ALUop, ALUsrc, ALUwidth;
	char Branch, MemRead, MemWrite;
	char RegWrite, MemtoReg;
	char MemWidth;

	/*
	Control Signals
	
	EXTop   0 unsigned
	        1 signed
	ALUop   0 add
	        1 mul
	        2 sub
	        3 div
	        4 shiftleft
	        5 shiftright
	        6 and
	        7 or
	        8 xor
	        9 mod
	        10 slt
	ALUsrc  0 rs1, rs2
	        1 rs1, imm
	        2 PC, 4
	        3 PC, imm
	        4 imm
	Branch  0 no branch
	        1 jalr
	        2 beq
	        3 bne
	        4 blt
	        5 bge
	        6 jal
	*/
	if(inst == 0) {
		memset(&ID_EX_old, 0, sizeof(ID_EX_old));
		printf("bubble\n");
		return false;
	}
	opcode = getbit(inst, 0, 6);
	rd = getbit(inst, 7, 11);
	fuc3 = getbit(inst, 12, 14);
	rs1 = getbit(inst, 15, 19);
	rs2 = getbit(inst, 20, 24);
	fuc7 = getbit(inst, 25, 31);

	//data risk
	if(opcode == OP_R || opcode == OP_RW || opcode == OP_S || opcode == OP_SB) {
		if((rs1 == ID_EX.Rd && ID_EX.Ctrl_WB_RegWrite == 1)
		 ||(rs1 == EX_MEM.Rd && EX_MEM.Ctrl_WB_RegWrite == 1)
		 ||(rs1 == MEM_WB.Rd && MEM_WB.Ctrl_WB_RegWrite == 1)
		 ||(rs2 == ID_EX.Rd && ID_EX.Ctrl_WB_RegWrite == 1)
		 ||(rs2 == EX_MEM.Rd && EX_MEM.Ctrl_WB_RegWrite == 1)
		 ||(rs2 == MEM_WB.Rd  && MEM_WB.Ctrl_WB_RegWrite == 1)) {
			memset(&ID_EX_old, 0, sizeof(ID_EX_old));
			IF_ID_old.inst = inst;
			IF_ID_old.PC = IF_ID.PC;
			PC = IF_ID.PC + 4;
			data_risk += 1;
			printf("bubble\n");
			return false;
		}
		else {
			printf("PC: %08llx     ", IF_ID.PC);
			printf("Inst: %08x  ", IF_ID.inst);
		}
	}
	else if(opcode == OP_AUI || opcode == OP_LUI || opcode == OP_JAL) {
	    printf("PC: %08llx     ", IF_ID.PC);
		printf("Inst: %08x  ", IF_ID.inst);
	}
	else if((rs1 == ID_EX.Rd && ID_EX.Ctrl_WB_RegWrite == 1)
		 ||(rs1 == EX_MEM.Rd && EX_MEM.Ctrl_WB_RegWrite == 1)
		 ||(rs1 == MEM_WB.Rd && MEM_WB.Ctrl_WB_RegWrite == 1)) {
		memset(&ID_EX_old, 0, sizeof(ID_EX_old));
		IF_ID_old.inst = inst;
		IF_ID_old.PC = IF_ID.PC;
		PC = IF_ID.PC + 4;
		data_risk += 1;
		printf("bubble\n");
		return false;
	}
	else {
		printf("PC: %08llx     ", IF_ID.PC);
		printf("Inst: %08x  ", IF_ID.inst);
	}
	if (opcode == OP_I || opcode == OP_IW || opcode == OP_L || opcode == OP_ECAL || opcode == OP_JALR) {
		imm12 = getbit(inst, 20, 31);
	}
	else if (opcode == OP_S) {
		imm12 = getbit(inst, 7, 11) + (getbit(inst, 25, 31) << 5);
	}
	else if (opcode == OP_SB) {
		imm12 = getbit(inst, 8, 11) + (getbit(inst, 25, 30) << 4) + (getbit(inst, 7, 7) << 10) + (getbit(inst, 31, 31) << 11);
	}
	else if (opcode == OP_AUI || opcode == OP_LUI) {
		imm20 = getbit(inst, 12, 31);
	}
	else if (opcode == OP_JAL) {
		imm20 = getbit(inst, 21, 30) + (getbit(inst, 20, 20) << 10) + (getbit(inst, 12, 19) << 11) + (getbit(inst, 31, 31) << 19);
	}

	if (opcode == OP_R || opcode == OP_RW) {
		EXTop = 0;

		ALUsrc = 0;
		ALUwidth = 64;
		Branch = 0;
		MemRead = 0;
		MemWrite = 0;
		RegWrite = 1;
		MemtoReg = 0;
		MemWidth = 0;

		if (fuc3 == 0x0 && fuc7 == 0x00)	    ALUop = 0;
		else if (fuc3 == 0x0 && fuc7 == 0x01)	ALUop = 1;
		else if (fuc3 == 0x0 && fuc7 == 0x20)	ALUop = 2;
		else if (fuc3 == 0x1 && fuc7 == 0x00)	ALUop = 4;
		else if (fuc3 == 0x1 && fuc7 == 0x01)	ALUop = 1;
		else if (fuc3 == 0x2 && fuc7 == 0x00)	ALUop = 10;
		else if (fuc3 == 0x4 && fuc7 == 0x00)	ALUop = 8;
		else if (fuc3 == 0x4 && fuc7 == 0x01)	ALUop = 3;
		else if (fuc3 == 0x5 && fuc7 == 0x00)	ALUop = 5;
		else if (fuc3 == 0x5 && fuc7 == 0x20)	ALUop = 5;
		else if (fuc3 == 0x6 && fuc7 == 0x00)	ALUop = 7;
		else if (fuc3 == 0x6 && fuc7 == 0x01)	ALUop = 9;
		else if (fuc3 == 0x7 && fuc7 == 0x00)   ALUop = 6;

		if (opcode == OP_RW) 
			ALUwidth = 32;

		if (1) {
			if (fuc3 == 0x0 && fuc7 == 0x00)	    printf("add");
			else if (fuc3 == 0x0 && fuc7 == 0x01)	printf("mul");
			else if (fuc3 == 0x0 && fuc7 == 0x20)	printf("sub");
			else if (fuc3 == 0x1 && fuc7 == 0x00)	printf("sll");
			else if (fuc3 == 0x1 && fuc7 == 0x01)	printf("mulh");
			else if (fuc3 == 0x2 && fuc7 == 0x00)	printf("slt");
			else if (fuc3 == 0x4 && fuc7 == 0x00)	printf("xor");
			else if (fuc3 == 0x4 && fuc7 == 0x01)	printf("div");
			else if (fuc3 == 0x5 && fuc7 == 0x00)	printf("srl");
			else if (fuc3 == 0x5 && fuc7 == 0x20)	printf("sra");
			else if (fuc3 == 0x6 && fuc7 == 0x00)	printf("or");
			else if (fuc3 == 0x6 && fuc7 == 0x01)	printf("rem");
			else if (fuc3 == 0x7 && fuc7 == 0x00)   printf("and");
			else printf("oops");
			if (opcode == OP_RW) printf("w");
			printf(" %s, %s, %s\n", Reg_name[rd], Reg_name[rs1], Reg_name[rs2]);
		}
	}
	else if (opcode == OP_L) {
		EXTop = 12;
		EXTsrc = imm12;

		ALUsrc = 1;
		ALUop = 0;
		ALUwidth = 64;
		Branch = 0;
		MemRead = 1;
		MemWrite = 0;
		RegWrite = 1;
		MemtoReg = 1;
		MemWidth = fuc3 + 1;

		if (1) {
			if (MemWidth == 1) printf("lb ");
			if (MemWidth == 2) printf("lh ");
			if (MemWidth == 3) printf("lw ");
			if (MemWidth == 4) printf("ld ");
			printf("%s, %lld(%s)\n", Reg_name[rd], ext_signed(EXTsrc, EXTop), Reg_name[rs1]);		
		}
	}
	else if (opcode == OP_I || opcode == OP_IW) {
		EXTop = 0;
		EXTsrc = imm12;

		ALUsrc = 1;
		ALUwidth = 64;
		Branch = 0;
		MemRead = 0;
		MemWrite = 0;
		RegWrite = 1;
		MemtoReg = 0;
		MemWidth = 0;

		if (fuc3 == 0x00) {
			ALUop = 0;
			EXTop = 12;
		}
		else if (fuc3 == 0x01)	ALUop = 4;
		else if (fuc3 == 0x02)	ALUop = 10;
		else if (fuc3 == 0x04)	ALUop = 8;
		else if (fuc3 == 0x05)	ALUop = 5;
		else if (fuc3 == 0x06)	ALUop = 7;
		else if (fuc3 == 0x07)	ALUop = 6;

		if (opcode == OP_IW)
			ALUwidth = 32;
		
		if (1) {
			if (fuc3 == 0x00)	printf("addi");
			else if (fuc3 == 0x01)	printf("slli");
			else if (fuc3 == 0x02)	printf("slti");
			else if (fuc3 == 0x04)	printf("xori");
			else if (fuc3 == 0x05) {
				if (fuc7 == 0x00)   printf("srli");
				else if (fuc7 == 0x20)   printf("srai");
			}
			else if (fuc3 == 0x06)	printf("ori");
			else if (fuc3 == 0x07)	printf("andi");

			if (opcode == OP_IW) printf("w");
			printf(" %s, %s, %lld\n", Reg_name[rd], Reg_name[rs1], ext_signed(EXTsrc, EXTop));
		}
	}
	else if (opcode == OP_JALR) {
		EXTop = 12;
		EXTsrc = imm12;

		ALUsrc = 2;
		ALUop = 0;
		ALUwidth = 64;
		Branch = 1;
		MemRead = 0;
		MemWrite = 0;
		RegWrite = 1;
		MemtoReg = 0;
		MemWidth = 0;

		if(1)
			printf("jalr %s, %s, %lld\n", Reg_name[rd], Reg_name[rs1], ext_signed(EXTsrc, EXTop));
	}
	else if (opcode == OP_S) {
		EXTop = 12;
		EXTsrc = imm12;

		ALUsrc = 1;
		ALUop = 0;
		ALUwidth = 64;
		Branch = 0;
		MemRead = 0;
		MemWrite = 1;
		RegWrite = 0;
		MemtoReg = 0;
		MemWidth = fuc3 + 1;

		if (1) {
			if (MemWidth == 1) printf("sb ");
			if (MemWidth == 2) printf("sh ");
			if (MemWidth == 3) printf("sw ");
			if (MemWidth == 4) printf("sd ");
			printf("%s, %lld(%s)\n", Reg_name[rs2], ext_signed(EXTsrc, EXTop), Reg_name[rs1]);
		}
	}

	else if (opcode == OP_SB) {
		EXTop = 13;
		EXTsrc = imm12 << 1;

		ALUsrc = 0;
		ALUop = 2;
		ALUwidth = 64;
		MemRead = 0;
		MemWrite = 0;
		RegWrite = 0;
		MemtoReg = 0;
		MemWidth = 0;

		if (fuc3 == 0x0)   Branch = 2;
		else if (fuc3 == 0x1)	Branch = 3;
		else if (fuc3 == 0x4)	Branch = 4;
		else if (fuc3 == 0x5)	Branch = 5;

		if (1) {
			if (fuc3 == 0x0)   printf("beq ");
			else if (fuc3 == 0x1)	printf("bne ");
			else if (fuc3 == 0x4)	printf("blt ");
			else if (fuc3 == 0x5)	printf("bge ");
			printf("%s, %s, %lld\n", Reg_name[rs1], Reg_name[rs2], ext_signed(EXTsrc, EXTop));
		}
	}
	else if (opcode == OP_AUI) {
		EXTop = 32;
		EXTsrc = imm20 << 12;

		ALUsrc = 3;
		ALUop = 0;
		ALUwidth = 64;
		Branch = 0;
		MemRead = 0;
		MemWrite = 0;
		RegWrite = 1;
		MemtoReg = 0;
		MemWidth = 0;
		printf("auipc %s, %lld\n", Reg_name[rd], ext_signed(EXTsrc, EXTop));
	}
	else if (opcode == OP_LUI) {
		EXTop = 32;
		EXTsrc = imm20 << 12;

		ALUsrc = 4;
		ALUop = 0;
		ALUwidth = 64;
		Branch = 0;
		MemRead = 0;
		MemWrite = 0;
		RegWrite = 1;
		MemtoReg = 0;
		MemWidth = 0;
		printf("lui %s, %lld\n", Reg_name[rd], ext_signed(EXTsrc, EXTop));
	}
	else if (opcode == OP_JAL) {
		EXTop = 21;
		EXTsrc = imm20 << 1;

		ALUsrc = 2;
		ALUop = 0;
		ALUwidth = 64;
		Branch = 6;
		MemRead = 0;
		MemWrite = 0;
		RegWrite = 1;
		MemtoReg = 0;
		MemWidth = 0;
		printf("jal %s, %lld\n", Reg_name[rd], ext_signed(EXTsrc, EXTop));
	}
	//write ID_EX_old
	ID_EX_old.Rd = rd;
	ID_EX_old.Rs1 = rs1;
	ID_EX_old.Rs2 = rs2;
	ID_EX_old.Imm = ext_signed(EXTsrc, EXTop);
	ID_EX_old.PC = IF_ID.PC;
	ID_EX_old.Reg_Rs1 = reg[rs1];
	ID_EX_old.Reg_Rs2 = reg[rs2];
	ID_EX_old.Ctrl_EX_ALUSrc = ALUsrc;
	ID_EX_old.Ctrl_EX_ALUOp = ALUop;
	ID_EX_old.Ctrl_EX_ALUWidth = ALUwidth;
	ID_EX_old.Ctrl_M_Branch = Branch;
	ID_EX_old.Ctrl_M_MemRead = MemRead;
	ID_EX_old.Ctrl_M_MemWrite = MemWrite;
	ID_EX_old.Ctrl_M_MemWidth = MemWidth;
	ID_EX_old.Ctrl_WB_RegWrite = RegWrite;
	ID_EX_old.Ctrl_WB_MemtoReg = MemtoReg;

	//data risk
	long long temp_PC = IF_ID.PC;
	if(Branch == 1)
		temp_PC = ((reg[rs1] + ID_EX_old.Imm) >> 1) << 1;
	else
		temp_PC = temp_PC + ID_EX_old.Imm;
	if(Branch != 0) {
		memset(&IF_ID_old, 0, sizeof(IF_ID_old));
		PC = temp_PC;
		wrongs += 1;
	}	
	return true;
}

//ִExecution
void EX()
{
	//read ID_EX
	int Rd = ID_EX.Rd;
	long long currentPC = ID_EX.PC;
	long long Imm = ID_EX.Imm;
	REG Reg_Rs1 = ID_EX.Reg_Rs1,
		Reg_Rs2 = ID_EX.Reg_Rs2;

	char ALUsrc = ID_EX.Ctrl_EX_ALUSrc,
		ALUop = ID_EX.Ctrl_EX_ALUOp,
		ALUwidth = ID_EX.Ctrl_EX_ALUWidth;

	char Branch = ID_EX.Ctrl_M_Branch;

	//choose ALU input number
	long long ra, rb;
	if (ALUsrc == 0) {
		ra = Reg_Rs1;
		rb = Reg_Rs2;
	}
	else if (ALUsrc == 1) {
		ra = Reg_Rs1;
		rb = Imm;
	}
	else if (ALUsrc == 2) {
		ra = currentPC;
		rb = 4;
	}
	else if (ALUsrc == 3) {
		ra = currentPC;
		rb = Imm;
	}
	else {
		ra = Imm;
		rb = 0;
	}
	if (ALUwidth == 32) {
		long long mask = 0xffffffff;
		ra = mask & ra;
		rb = mask & rb;
	}

	//alu calculate
	long long ALU_out = 0;
	if (ALUop == 0) {
		ALU_out = ra + rb;
	}
	else if (ALUop == 1) {
		ALU_out = ra * rb;
		total_cycle += 1;
	}
	else if (ALUop == 2) {
		ALU_out = ra - rb;
	}
	else if (ALUop == 3) {
		ALU_out = ra / rb;
		total_cycle += 39;
	}
	else if (ALUop == 4) {
		ALU_out = ra << rb;
	}
	else if (ALUop == 5) {
		ALU_out = ra >> rb;
	}
	else if (ALUop == 6) {
		ALU_out = ra & rb;
	}
	else if (ALUop == 7) {
		ALU_out = ra | rb;
	}
	else if (ALUop == 8) {
		ALU_out = ra ^ rb;
	}
	else if (ALUop == 9) {
		ALU_out = ra % rb;
		total_cycle += 39;
	}
	else if (ALUop == 10) {
		if ((long long)ra < (long long)rb)
			ALU_out = 1;
		else
			ALU_out = 0;
	}
	if (ALUwidth == 32) {
		ALU_out = ext_signed(ALU_out, 32);
	}
	if ((Branch == 2 && ALU_out != 0) || (Branch == 3 && ALU_out == 0) || 
	    (Branch == 4 && ALU_out >= 0) || (Branch == 5 && ALU_out < 0)) {
		memset(&IF_ID_old, 0, sizeof(IF_ID_old));
		memset(&ID_EX_old, 0, sizeof(ID_EX_old));
		PC = currentPC + 4; 
		wrongs += 1;
	}

	//write EX_MEM_old
	EX_MEM_old.Rd = Rd;
	EX_MEM_old.ALU_out = ALU_out;
	EX_MEM_old.PC = currentPC;
	EX_MEM_old.Reg_Rs1 = Reg_Rs1;
	EX_MEM_old.Reg_Rs2 = Reg_Rs2;

	EX_MEM_old.Ctrl_M_Branch = Branch;
	EX_MEM_old.Ctrl_M_MemWrite = ID_EX.Ctrl_M_MemWrite;
	EX_MEM_old.Ctrl_M_MemRead = ID_EX.Ctrl_M_MemRead;
	EX_MEM_old.Ctrl_M_MemWidth = ID_EX.Ctrl_M_MemWidth;

	EX_MEM_old.Ctrl_WB_RegWrite = ID_EX.Ctrl_WB_RegWrite;
	EX_MEM_old.Ctrl_WB_MemtoReg = ID_EX.Ctrl_WB_MemtoReg;
}

void MEM()
{
	//read EX_MEM
	int rd = EX_MEM.Rd;
	long long ALU_out = EX_MEM.ALU_out;
	REG currentPC = EX_MEM.PC,
		Reg_Rs1 = EX_MEM.Reg_Rs1,
		Reg_Rs2 = EX_MEM.Reg_Rs2;

	char Branch = EX_MEM.Ctrl_M_Branch,
		MemWrite = EX_MEM.Ctrl_M_MemWrite,
		MemRead = EX_MEM.Ctrl_M_MemRead,
		MemWidth = EX_MEM.Ctrl_M_MemWidth;
	
	//read / write memory
	char Memout[256] = { 0 };
	unsigned long long Mem_read;
	int hit, time;
	if (MemRead == 1) {
		if (MemWidth == 1) {
			l1.HandleRequest(ALU_out, 1, 1, Memout, hit, time);
			Mem_read = ext_signed(*(unsigned int*)Memout, 8);
		}
		else if (MemWidth == 2) {
			l1.HandleRequest(ALU_out, 2, 1, Memout, hit, time);
			Mem_read = ext_signed(*(unsigned int*)Memout, 16);
		}
		else if (MemWidth == 3) {
			l1.HandleRequest(ALU_out, 4, 1, Memout, hit, time);
			Mem_read = ext_signed(*(unsigned int*)Memout, 32);
		}
		else if (MemWidth == 4) {
			l1.HandleRequest(ALU_out, 8, 1, Memout, hit, time);
			Mem_read = *(unsigned long long*)Memout;
		}
	}
	else if (MemWrite == 1) {
		if (MemWidth == 1) {
			l1.HandleRequest(ALU_out, 1, 0, (char*)&Reg_Rs2, hit, time);
		}
		else if (MemWidth == 2) {
			l1.HandleRequest(ALU_out, 2, 0, (char*)&Reg_Rs2, hit, time);
		}
		else if (MemWidth == 3) {
			l1.HandleRequest(ALU_out, 4, 0, (char*)&Reg_Rs2, hit, time);
		}
		else if (MemWidth == 4) {
			l1.HandleRequest(ALU_out, 8, 0, (char*)&Reg_Rs2, hit, time);
		}
	}
	
	//write MEM_WB_old
	MEM_WB_old.Mem_read = Mem_read;
	MEM_WB_old.ALU_out = ALU_out;
	MEM_WB_old.Rd = rd;
	MEM_WB_old.PC = currentPC;
	MEM_WB_old.Ctrl_WB_RegWrite = EX_MEM.Ctrl_WB_RegWrite;
	MEM_WB_old.Ctrl_WB_MemtoReg = EX_MEM.Ctrl_WB_MemtoReg;
}

//Write Back
void WB()
{
	//read MEM_WB
	char RegWrite = MEM_WB.Ctrl_WB_RegWrite,
		MemtoReg = MEM_WB.Ctrl_WB_MemtoReg;
	int rd = MEM_WB.Rd;
	REG ALU_out = MEM_WB.ALU_out,
		Mem_read = MEM_WB.Mem_read;
	
	//write reg
	if (RegWrite == 1) {
		if (MemtoReg == 0)
			reg[rd] = ALU_out;
		else
			reg[rd] = Mem_read;
	}
	if(MEM_WB.PC != 0)
	    total_inst += 1;
}
