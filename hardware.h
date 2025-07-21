#ifndef HARDWARE_H
#define HARDWARE_H
#include <stdint.h>

#define MAX_MEMORY (1 << 16)

enum reg_index
{
    R0 = 0,
    R1,
    R2,
    R3,
    R4,
    R5,
    R6, /* Supervisro/User stack pointer */
    R7, /* Temporary register for the PC register */
    R_PC,
    R_PSR,  /*Process status register */
    R_COUNT /* The amount of registers */
};

enum device_registers
{
    KBSR = 0xFE00, /* Keyboard status register */
    KBDR = 0xFE02, /* Keyboard data register */
    DSR = 0xFE04,  /* Diplay status register */
    DDR = 0xFE06,  /* Display data register */
    MCR = 0xFFFE   /* Machine control register */
};

enum operations {
    OP_BR = 0,
    OP_ADD,
    OP_LD,
    OP_ST,
    OP_JSR,
    OP_AND,
    OP_LDR,
    OP_STR,
    OP_RTI,
    OP_NOT,
    OP_LDI,
    OP_STI,
    OP_JMP,
    OP_RESERVED,
    OP_LEA,
    OP_TRAP,
    OP_CONUT
};

enum condition_codes
{
    POS = (uint16_t)(1 << 0),
    ZRO = (uint16_t)(1 << 1),
    NEG = (uint16_t)(1 << 2)
};

enum system_calls{
    TRAP_GETC = 0x20,
    TRAP_OUT = 0x21,
    TRAP_PUTS = 0x22,
    TRAP_IN = 0x23,
    TRAP_PUTSP = 0x24,
    TRAP_HALT = 0x25
};


/*Assembly Instructions*/
void ADD(uint16_t instr);
void AND(uint16_t instr);
void BR(uint16_t instr);
void JMP(uint16_t instr);
void JSR(uint16_t instr);
void LD(uint16_t instr);
void LDI(uint16_t instr);
void LDR(uint16_t instr);
void LEA(uint16_t instr);
void NOT(uint16_t instr);
void RTI();
void ST(uint16_t instr);
void STI(uint16_t instr);
void STR(uint16_t instr);
void TRAP(uint16_t instr);
void RESERVED();

/*Trap Routines*/
void GETC();
void OUT_t();
void PUTS();
void IN_t();
void PUTSP();
void HALT();

/* Helper functions */
uint16_t SEXT(uint16_t oprand, uint16_t digit_count);
void SETCC(uint16_t value);
void mem_write(uint16_t address, uint16_t val);
uint16_t mem_read(uint16_t address);
void disable_input_buffering();
void restore_input_buffering();
uint16_t check_key();
void handle_interrupt(int signal);

#endif