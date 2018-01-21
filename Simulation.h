#include <iostream>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "Reg_def.h"

#define OP_R     0x33
#define OP_RW    0x3B
#define OP_L     0x03
#define OP_I     0x13
#define OP_IW    0x1B
#define OP_JALR  0x67
#define OP_ECAL  0x73
#define OP_S     0x23
#define OP_SB    0x63
#define OP_AUI   0x17
#define OP_LUI   0x37
#define OP_JAL   0x6f

#define MAX 100000000

//main memory
unsigned char memory[MAX] = {0};

//registers
REG reg[32] = {0};

//PC
long long PC = 0;

//max 64-bit unsigned int
unsigned long long MAX_INT = 0xffffffffffffffff;

void load_memory();

void simulate();

void IF();

bool ID();

void EX();

void MEM();

void WB();

long long int ext_signed(unsigned int src,int bit);

unsigned int getbit(unsigned int inst, int s,int e);

unsigned int getbit(unsigned int inst,int s,int e)
{
	return (inst << (31 - e)) >> (31 - e + s);
}

long long int ext_signed(unsigned int src,int bit)
{
    if(bit == 0)
        return src;
    else {
        unsigned long mask = 1;
        mask = mask << (bit - 1);
        mask = mask & src;
        if(mask == 0)
            return src;
        else {
			return src + (MAX_INT << bit);
        }
    }
}