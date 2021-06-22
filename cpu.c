#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

/* Hardware Structures */
struct reg_file {
	uint8_t a;
	uint8_t x;
	uint8_t y;
	uint16_t pc;
	uint8_t sp;
	uint8_t status;
};

uint8_t * ram;
uint8_t * rom; // 32 Kb is 32768 bytes
struct reg_file reg_file;

/* Address Modes */
typedef enum {
	acc_mode,
	abs_mode,
	absx_mode,
	absy_mode,
	imm_mode,
	ind_mode,
	indx_mode,
	indy_mode,
	rel_mode,
	zero_mode,
	zerox_mode,
	zeroy_mode
} addr_mode;

/* Utility functions */

// Gets the address based on the mode that is passed in
uint16_t get_addr(uint16_t addr, addr_mode mode) {
	switch (mode) {

		case abs_mode:
			return addr;

		case absx_mode:
			return reg_file.x + addr;

		case absy_mode:
			return reg_file.y + addr;

		case indx_mode:
			return ram[((addr & 0xFF) + reg_file.x) & 0xFF]
				+ ram[(((addr & 0xFF) + reg_file.x + 1) & 0xFF) << 8];

		case indy_mode:
			return ram[addr] + ram[((addr + 1) & 0xFF) << 8 + reg_file.y]; 

		case zero_mode:
			return addr & 0xFF;

		case zerox_mode:
			return (reg_file.x + (addr & 0xFF)) & 0xFF;

		case zeroy_mode:
			return (reg_file.y + (addr & 0xFF)) & 0xFF;

	}
}

/* FLAGS */

# define STATUS_C 0
# define STATUS_Z 1
# define STATUS_I 2
# define STATUS_D 3
# define STATUS_B 4
# define STATUS_V 6
# define STATUS_N 7

void set_flag(int place) {
	reg_file.status = reg_file.status | (1 << place);
}

void clear_flag(int place) {
	reg_file.status = reg_file.status & ~(1 << place); 
}


void step_pc(addr_mode mode) {

	if (mode == abs_mode || 
	    mode == absx_mode || 
	    mode == absy_mode) {

		reg_file.pc += 3;

	} else if (mode == 0) {

		reg_file.pc += 1;

	} else {

		reg_file.pc += 2;

	}
}


/* INSTRUCTIONS */

/* Load and Store Functions */

void lda(addr_mode mode) {
	
	if (mode == imm_mode) {
		reg_file.a = rom[reg_file.pc + 1];
	} else {
		reg_file.a = ram[get_addr((uint16_t) rom[reg_file.pc + 1], mode)];
	}

	if (reg_file.a >> 7 == 1) {
	        clear_flag(STATUS_Z);	
		set_flag(STATUS_N);
	} else if (reg_file.a == 0) {
		clear_flag(STATUS_N);
		set_flag(STATUS_Z);
	} else {
		clear_flag(STATUS_Z);
		clear_flag(STATUS_N);
	}

	step_pc(mode);	
}

void sta(addr_mode mode) {
	ram[get_addr((uint16_t) rom[reg_file.pc + 1], mode)] = reg_file.a;
}

/* INITIALIZATION */

void load_rom(char * filename) {
	FILE * file_ptr = fopen(filename, "r");
	if (file_ptr == NULL) {
		printf("%s", "load_rom: file ptr is null");
	}
	fread(rom, sizeof(rom), 1, file_ptr);
	fclose(file_ptr);	
}

void load_test_rom(uint8_t test_rom[]) {
	
	for (int i = 0; i < sizeof(test_rom); i++) {
		printf("%x", test_rom[i]);
	}
}


/* LOOKUP TABLES */

addr_mode get_alu_mode(void) {
	switch (rom[reg_file.pc] & 0x1F) {
		case(0x1): return indx_mode;
		case(0x5): return zero_mode;
		case(0x9): return imm_mode;
		case(0xD): return abs_mode;
		case(0x11): return indx_mode;
		case(0x15): return zerox_mode;
		case(0x19): return absy_mode;
		case(0x1D): return absx_mode;
	}
}

/* DEBUGGING */

void print_reg_file(void) {
        printf("%s", "reg_file \n");
        printf("reg_file.a %x\n", reg_file.a);
        printf("reg_file.status %x\n", reg_file.status);
}

void execute_alu(void) {
	switch(rom[reg_file.pc] & 0xE0) {
		case(0xA0): lda(get_alu_mode());
		case(0x80): sta(get_alu_mode());
	}	
}

void execute(void) {
	while (reg_file.pc < sizeof(rom)) {
		switch (rom[reg_file.pc] & 0x3) {
			/* ALU CASES */
			case (0x1): execute_alu();
		}
		//print_reg_file();
	}
}

int main(int argc, char * argv[]) {
	load_test_rom();
	//execute();
	return 0;
}
