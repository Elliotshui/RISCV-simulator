
#include "Read_Elf.h"
#pragma warning (disable:4996)

FILE *elf = NULL;
Elf64_Ehdr elf64_hdr;

extern unsigned char memory[];

//Program headers
unsigned int padr = 0;
unsigned int psize = 0;
unsigned int pnum = 0;

//Section Headers
unsigned int sadr = 0;
unsigned int ssize = 0;
unsigned int snum = 0;

//Symbol table
unsigned int symnum = 0;
unsigned int symadr = 0;
unsigned int symsize = 0;

//shstrtab index
unsigned int idx = 0;

//Section headers string table 
unsigned int shadr = 0;
unsigned int shsize = 0;
unsigned char* shtable = NULL;

//Symbol name string table
unsigned int stradr = 0;
unsigned int strsize = 0;
unsigned char* strtable = NULL;

bool open_file()
{
	char name[40];
	printf("Please input the file name: \n");
	scanf("%s", name);
	file = fopen(name, "rb");
	elf = fopen(strcat(name, "_info"), "w");
	return true;
}

void read_elf()
{
	if(!open_file())
		return ;
	read_elf_header();
	read_program_header();
	read_elf_sections();
	//read_Phdr();
	read_symtable();
	close_file();
}

void read_elf_header()
{
	fread(&elf64_hdr,1,sizeof(elf64_hdr),file);
	fprintf(elf, " ------ Elf header ------\n");
	fprintf(elf, " Magic Number: ");
	for(int i = 0; i < 16; ++i) {
		fprintf(elf, "%02x ", elf64_hdr.e_ident[i]);
	}
	fprintf(elf, "\n");
	fprintf(elf, " Class:  ELFCLASS32\n");	
	fprintf(elf, " Data:  little-endian\n");	
	fprintf(elf, " Version:  %u\n", elf64_hdr.e_ident[EI_VERSION]);
	fprintf(elf, " OS/ABI:  System V ABI\n");
	fprintf(elf, " ABI Version:  %u\n",elf64_hdr.e_ident[EI_ABIVERSION]);
	fprintf(elf, " Type:  %u\n", *(unsigned int*)elf64_hdr.e_type.b);
	fprintf(elf, " Machine:  %u\n", *(unsigned short*)elf64_hdr.e_machine.b);
	fprintf(elf, " Version:  %u\n", *(unsigned short*)elf64_hdr.e_version.b);
	fprintf(elf," Entry point address:  0x%x\n", entry);
	padr = *(unsigned long*)elf64_hdr.e_phoff.b;
	fprintf(elf," Start of program headers:  %u bytes into file\n", padr);
	sadr = *(unsigned long*)elf64_hdr.e_shoff.b;
	fprintf(elf," Start of section headers:  %u bytes into file\n", sadr);
	fprintf(elf," Flags:  0x%x\n", *(unsigned int*)elf64_hdr.e_flags.b);
	fprintf(elf," Size of this header:  %u Bytes\n", *(unsigned short*)elf64_hdr.e_ehsize.b);
	psize = *(unsigned short*)elf64_hdr.e_phentsize.b;
	fprintf(elf," Size of program headers:  %u Bytes\n", psize);
	pnum = *(unsigned short*)elf64_hdr.e_phnum.b;
	fprintf(elf," Number of program headers:  %u \n", pnum);
	ssize = *(unsigned short*)elf64_hdr.e_shentsize.b;	
	fprintf(elf," Size of section headers:  %u Bytes\n", ssize);
	snum = *(unsigned short*)elf64_hdr.e_shnum.b;
	fprintf(elf," Number of section headers:  %u \n", snum);
	idx = *(unsigned short*)elf64_hdr.e_shstrndx.b;
	fprintf(elf," Section header string table index:  %u\n", idx);
	fprintf(elf, "\n");
}

void read_program_header()
{
	Elf64_Phdr elf64_phdr;
	fprintf(elf, " ------ Program header ------\n");
	fprintf(elf, "        addr        offset    size      type\n");
	long long offset, vaddr, memsz;
	int type;
    for(int c = 0; c < pnum; ++c) {
		fprintf(elf," [%03d] ",c);
		fseek(file, padr + c * psize, SEEK_SET);
		fread(&elf64_phdr, 1, sizeof(Elf64_Phdr), file);
		type = *(unsigned int*)elf64_phdr.p_type.b;
		offset = *(unsigned long*)elf64_phdr.p_offset.b;
		vaddr = *(unsigned long*)elf64_phdr.p_vaddr.b;
		memsz = *(unsigned long*)elf64_phdr.p_memsz.b;
		fprintf(elf, " %08lx    ", vaddr);
		fprintf(elf, "%06d    ", offset);
		fprintf(elf, "%06d    ", memsz);
		fprintf(elf, "%d\n", type);
	}
	fprintf(elf, "\n");
}

void read_elf_sections()
{
	//determine string table offset
	Elf64_Shdr elf64_shdr;
	fseek(file, sadr + ssize * idx, SEEK_SET);
	fread(&elf64_shdr, 1, sizeof(Elf64_Shdr), file);
	shadr = *(unsigned long*)elf64_shdr.sh_offset.b;
	shsize = *(unsigned long*)elf64_shdr.sh_size.b;
	shtable = new unsigned char[shsize + 1];
	fseek(file, shadr, SEEK_SET);
	fread(shtable, 1, shsize, file);
	
	fprintf(elf, " ------ Section header ------\n");
	fprintf(elf, "        addr        offset    size      name\n");
	for(int c = 0;c < snum;c++)
	{
		fprintf(elf," [%03d] ",c);	
		fseek(file, ssize * c + sadr, SEEK_SET);
		fread(&elf64_shdr, 1, sizeof(Elf64_Shdr), file);
		unsigned int noff = *(unsigned int*)elf64_shdr.sh_name.b,
					 soff = *(unsigned long*)elf64_shdr.sh_offset.b,
					 secsize = *(unsigned long*)elf64_shdr.sh_size.b;
		unsigned long virtualadr = *(unsigned long*)elf64_shdr.sh_addr.b;
		char* name = (char*)shtable + noff;
		fprintf(elf, " %08lx    ", virtualadr);
		fprintf(elf, "%06d    ", soff);
		fprintf(elf, "%06d    ", secsize);
		fprintf(elf, "%s\n", name);

		if(strcmp(name, ".symtab") == 0) {
			symadr = *(unsigned long*)elf64_shdr.sh_offset.b;
			symsize = *(unsigned long*)elf64_shdr.sh_entsize.b;
			symnum = *(unsigned long*)elf64_shdr.sh_size.b / symsize;
		}
		if(strcmp(name, ".strtab") == 0) {
			stradr = *(unsigned long*)elf64_shdr.sh_offset.b;
			strsize = *(unsigned long*)elf64_shdr.sh_size.b;
			strtable = new unsigned char[strsize + 1];
			fseek(file, stradr, SEEK_SET);
			fread(strtable, 1, strsize, file);
		}
		if(virtualadr != 0 && strcmp(name, ".bss") != 0 && strcmp(name, ".sbss") != 0) {
		    fseek(file, soff, SEEK_SET);
	        fread(&memory[virtualadr], 1, secsize, file);
		}
	}
	fprintf(elf, "\n");
}
/*
void read_Phdr()
{
	Elf64_Phdr elf64_phdr;
	fprintf(elf, " ------ Program header ------");
	for(int c=0;c<pnum;c++)
	{
		fprintf(elf," [%3d]\n",c);	
		fseek(file, padr + c * psize, SEEK_SET);	
		fread(&elf64_phdr,1,sizeof(elf64_phdr),file);
		fprintf(elf," Type:   ");
		
		fprintf(elf," Flags:   ");
		
		fprintf(elf," Offset:   ");
		
		fprintf(elf," VirtAddr:  ");
		
		fprintf(elf," PhysAddr:   ");

		fprintf(elf," FileSiz:   ");

		fprintf(elf," MemSiz:   ");
		
		fprintf(elf," Align:   ");
	}
}
*/

void read_symtable()
{
	Elf64_Sym elf64_sym;
	fprintf(elf, " ------ Symtable ------\n");
	fprintf(elf, "        value       size      name\n");            
	for(int c = 0;c < symnum;c++)
	{
		fprintf(elf," [%03d] ",c);
		fseek(file, symadr + c * symsize, SEEK_SET);
		fread(&elf64_sym,1,sizeof(elf64_sym),file);
		unsigned int noff = *(unsigned int*)elf64_sym.st_name.b;
		unsigned int val = *(unsigned long*)elf64_sym.st_value.b;
		unsigned int size = *(unsigned long*)elf64_sym.st_size.b;
		char *name = (char*)strtable + noff;
		fprintf(elf, " %08x    ", val);
		fprintf(elf, "%06d    ", size);
		fprintf(elf, "%s\n", name);
		
		if(strcmp(name, "main") == 0) {
			madr = val;
			endPC = madr + size;
		}
		if(strcmp(name, "__global_pointer$") == 0) {
		    gp = val;
		}
		if(strcmp(name, "_gp") == 0) {
		    gp = val;
		}
	}
	fprintf(elf, "\n");
}

void close_file() {
	fclose(elf);
	fclose(file);
	delete [] shtable;
	delete [] strtable;
}
