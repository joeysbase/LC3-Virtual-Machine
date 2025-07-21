#include <stdio.h>
#include <signal.h>

/* For handling input */
#include <Windows.h>
#include <conio.h>

#include "hardware.h"

uint16_t memory[MAX_MEMORY] = {0};
uint16_t registers[R_COUNT] = {0};
int running = 1;

HANDLE hStdin = INVALID_HANDLE_VALUE;
DWORD fdwMode, fdwOldMode;

uint16_t SEXT(uint16_t oprand, uint16_t digit_count)
{
    if ((oprand >> (digit_count - 1)) & 1) /* Turn off every bit except the lowest bit*/
    {
        oprand |= (0xFFFF << digit_count);
    }
    return oprand;
}

void SETCC(uint16_t value)
{
    uint16_t switcher = 0xFFF8;
    registers[R_PSR] &= switcher;

    if (value == 0)
    {
        registers[R_PSR] |= ZRO;
    }
    else if (value >> 0xF)
    {
        registers[R_PSR] |= NEG;
    }
    else
    {
        registers[R_PSR] |= POS;
    }
}

/* Codes copy-paste from justinmeiners tutorial. Start */
/* The following codes ensure the keyboard inputs function properly in windows */
void disable_input_buffering()
{
    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(hStdin, &fdwOldMode);     /* save old mode */
    fdwMode = fdwOldMode ^ ENABLE_ECHO_INPUT /* no input echo */
            ^ ENABLE_LINE_INPUT;           /* return when one or
                                                  more characters are available */
    SetConsoleMode(hStdin, fdwMode);         /* set new mode */
    FlushConsoleInputBuffer(hStdin);         /* clear buffer */
}

void restore_input_buffering()
{
    SetConsoleMode(hStdin, fdwOldMode);
}

void handle_interrupt(int signal)
{
    restore_input_buffering();
    printf("\n");
    exit(-2);
}

uint16_t check_key()
{
    return WaitForSingleObject(hStdin, 1000) == WAIT_OBJECT_0 && _kbhit();
}

void mem_write(uint16_t address, uint16_t val)
{
    memory[address] = val;
}

uint16_t mem_read(uint16_t address)
{
    if (address == KBSR)
    {
        if (check_key())
        {
            memory[KBSR] = (1 << 15);
            memory[KBDR] = getchar();
        }
        else
        {
            memory[KBSR] = 0;
        }
    }
    return memory[address];
}
/* Codes copy-paste from justinmeiners tutorial. End */

void ADD(uint16_t instr)
{
    uint16_t DR = (instr >> 9) & 0x7;
    uint16_t SR1 = (instr >> 6) & 0x7;
    if ((instr >> 5) & 1)
    {
        uint16_t imm5 = instr & 0x1F;
        registers[DR] = registers[SR1] + SEXT(imm5, 5);
    }
    else
    {
        uint16_t SR2 = instr & 0x7;
        registers[DR] = registers[SR1] + registers[SR2];
    }
    SETCC(registers[DR]);
}

void AND(uint16_t instr)
{
    uint16_t DR = (instr >> 9) & 0x7;
    uint16_t SR1 = (instr >> 6) & 0x7;

    if ((instr >> 5) & 1)
    {
        uint16_t imm5 = instr & 0x1F;
        registers[DR] = registers[SR1] & SEXT(imm5, 5);
    }
    else
    {
        uint16_t SR2 = instr & 0x7;
        registers[DR] = registers[SR1] & registers[SR2];
    }
    SETCC(registers[DR]);
}

void BR(uint16_t instr)
{
    uint16_t NZP = registers[R_PSR] & 0x7;
    uint16_t nzp = (instr >> 9) & 0x7;

    if (NZP & nzp)
    {
        uint16_t PCoffset9 = instr & 0x1FF;
        registers[R_PC] += SEXT(PCoffset9, 9);
    }
}

void JMP(uint16_t instr)
{
    uint16_t BaseR = (instr >> 6) & 0x7;
    registers[R_PC] = registers[BaseR];
}

void JSR(uint16_t instr)
{
    registers[R7] = registers[R_PC];
    if ((instr >> 11) & 1)
    {
        uint16_t PCoffset11 = instr & 0x7FF;
        registers[R_PC] += SEXT(PCoffset11, 11);
    }
    else
    {
        uint16_t BaseR = (instr >> 6) & 0x7;
        registers[R_PC] = registers[BaseR];
    }
}

void LD(uint16_t instr)
{
    uint16_t DR = (instr >> 9) & 0x7;
    uint16_t PCoffset9 = instr & 0x1FF;
    registers[DR] = mem_read(registers[R_PC] + SEXT(PCoffset9, 9));
    SETCC(registers[DR]);
}

void LDI(uint16_t instr)
{
    uint16_t DR = (instr >> 9) & 0x7;
    uint16_t PCoffset9 = instr & 0x1FF;
    registers[DR] = mem_read(mem_read(registers[R_PC] + SEXT(PCoffset9, 9)));
    SETCC(registers[DR]);
}

void LDR(uint16_t instr)
{
    uint16_t DR = (instr >> 9) & 0x7;
    uint16_t offset6 = instr & 0x3F;
    uint16_t BaseR = (instr >> 6) & 0x7;
    registers[DR] = mem_read(registers[BaseR] + SEXT(offset6, 6));
    SETCC(registers[DR]);
}

void LEA(uint16_t instr)
{
    uint16_t DR = (instr >> 9) & 0x7;
    uint16_t PCoffset9 = instr & 0x1FF;
    registers[DR] = registers[R_PC] + SEXT(PCoffset9, 9);
    SETCC(registers[DR]);
}

void NOT(uint16_t instr)
{
    uint16_t DR = (instr >> 9) & 0x7;
    uint16_t SR = (instr >> 6) & 0x7;
    registers[DR] = ~(registers[SR]);
    SETCC(registers[DR]);
}

void RTI()
{
    uint16_t temp = 0;
    if (((registers[R_PSR] >> 15) & 1) == 0)
    {
        registers[R_PC] = memory[registers[R6]];
        registers[R6] += 1;
        temp = registers[R6];
        registers[R6] += 1;
        registers[R_PSR] = temp;
    }
}

void ST(uint16_t instr)
{
    uint16_t SR = (instr >> 9) & 0x7;
    uint16_t PCoffset9 = instr & 0x1FF;
    mem_write(registers[R_PC] + SEXT(PCoffset9, 9), registers[SR]);
}

void STI(uint16_t instr)
{
    uint16_t SR = (instr >> 9) & 0x7;
    uint16_t PCoffset9 = instr & 0x1FF;
    mem_write(mem_read(registers[R_PC] + SEXT(PCoffset9, 9)), registers[SR]);
}

void STR(uint16_t instr)
{
    uint16_t SR = (instr >> 9) & 0x7;
    uint16_t BaseR = (instr >> 6) & 0x7;
    uint16_t offset6 = instr & 0x3F;
    mem_write(registers[BaseR] + SEXT(offset6, 6), registers[SR]);
}

void TRAP(uint16_t instr)
{
    uint16_t trapvect8 = instr & 0xFF;

    /* Use C function calls instead */
    // registers[R7] = registers[R_PC];
    // registers[R_PC] = memory[trapvect8];

    switch (trapvect8)
    {
    case TRAP_GETC:
        GETC();
        break;
    case TRAP_OUT:
        OUT_t();
        break;
    case TRAP_PUTS:
        PUTS();
        break;
    case TRAP_IN:
        IN_t();
        break;
    case TRAP_PUTSP:
        PUTSP();
        break;
    case TRAP_HALT:
        HALT();
        break;
    default:
        fputs("Exception: BAD TRAP CODE", stdout);
        HALT();
        break;
    }
}

void RESERVED()
{
}

/* Trap routines */
void GETC()
{
    registers[R0] = (uint16_t)getchar();
    SETCC(registers[R0]);
}

void OUT_t()
{
    putchar((char)registers[R0]);
    fflush(stdout);
}

void PUTS()
{
    uint16_t address = registers[R0];
    // printf("address->%d\n", address);
    // printf("memory[address]->%d\n", memory[address]);
    while (memory[address] != 0)
    {
        putchar((char)memory[address++]);
    }
}

void IN_t()
{
    fputs("Enter a character (press \"Enter\" to confirm your input)\n> ", stdout);
    registers[R0] = (uint16_t)getchar();
    SETCC(registers[R0]);
}

void PUTSP()
{
    uint16_t address = registers[R0];
    while (memory[address] != 0)
    {
        uint16_t content = memory[address++];
        putchar((char)(content & 0xFF));
        if ((content >> 8) & 0xFF)
        {
            putchar((char)((content >> 8) & 0xFF));
        }
    }
}

void HALT()
{
    fputs("The process was halted.", stdout);
    running = 0;
}

