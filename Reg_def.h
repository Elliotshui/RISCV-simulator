typedef unsigned long long REG;

struct IFID{
	unsigned int inst;
	long long PC;
}IF_ID, IF_ID_old;


struct IDEX{
	int Rd, Rs1, Rs2;
	long long PC;
	long long Imm;
	REG Reg_Rs1, Reg_Rs2;

	char Ctrl_EX_ALUSrc;
	char Ctrl_EX_ALUOp;
	char Ctrl_EX_ALUWidth;

	char Ctrl_M_Branch;
	char Ctrl_M_MemWrite;
	char Ctrl_M_MemRead;
	char Ctrl_M_MemWidth;

	char Ctrl_WB_RegWrite;
	char Ctrl_WB_MemtoReg;

}ID_EX,ID_EX_old;

struct EXMEM{
	int Rd;
	REG ALU_out, PC;
	REG Reg_Rs1, Reg_Rs2;

	char Ctrl_M_Branch;
	char Ctrl_M_MemWrite;
	char Ctrl_M_MemRead;
	char Ctrl_M_MemWidth;

	char Ctrl_WB_RegWrite;
	char Ctrl_WB_MemtoReg;

}EX_MEM,EX_MEM_old;

struct MEMWB{
	unsigned long long Mem_read;
	REG ALU_out;
	long long PC;
	int Rd;
		
	char Ctrl_WB_RegWrite;
	char Ctrl_WB_MemtoReg;

}MEM_WB,MEM_WB_old;