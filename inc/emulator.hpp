#ifndef _LINKER__STRUCTS__
#define _LINKER__STRUCTS__

#include <iostream>
#include <string>
#include <vector>
using namespace std;

typedef short reg;

//REGISTER ENUM
enum{r0, r1, r2, r3, r4, r5, r6, r7, r8};
enum{sp = 6, pc = 7};


//MASKS FOR BITS IN PSW

#define Z_MASK 0x1
#define O_MASK (0x1 << 1)
#define C_MASK (0x1 << 2)
#define N_MASK (0x1 << 3)
#define TIMER_MASK (0x1 << 13)
#define TERMINAL_MASK (0x1 << 14)
#define INTERRUPT_MASK (0x1 << 15)


//OC
#define HALT 0x00
#define IRET 0x20
#define RET 0x40
#define INT 0x10
#define PUSH_OR_STR 0xb0
#define POP_OR_LDR 0xa0
#define NOT 0x80
#define XCHG 0x60
#define ADD 0x70
#define SUB 0x71
#define MUL 0x72
#define DIV 0x73
#define CMP 0x74
#define AND 0x81
#define OR 0x82
#define XOR 0x83
#define TEST 0x84
#define SHL 0x90
#define SHR 0x91
#define CALL 0x30
#define JMP 0x50
#define JEQ 0x51
#define JNE 0x52
#define JGT 0x53

#define DESTINATION_REG_MASK 0xf0
#define SOURCE_REG_MASK 0xf

#define LOWER_BYTE 0xff
#define HIGHER_BYTE 0xff00


#define IMMEDIATE 0
#define PC_RELATIVE_OR_REG_IND_DISP 0x03
#define MEM_DIR 0x04
#define REG_DIR 0x01
#define REG_IND 0x02
#define PC_REL_ADDR 0x05


struct cpu_registers{
  reg regs[8];
  reg psw;
};



#endif
