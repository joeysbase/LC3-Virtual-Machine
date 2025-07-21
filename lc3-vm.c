#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <stdlib.h>

#include "hardware.h"

extern uint16_t memory[MAX_MEMORY];
extern uint16_t registers[R_COUNT];
extern int running;

int load_file(const char *file_path);
void read_file(FILE *file);
uint16_t swap16(uint16_t x);
void show_binary(uint16_t instr);
void show_asm(uint16_t op_code, uint16_t instr);

int main(int argc, char *args[])
{
    signal(SIGINT, handle_interrupt);
    disable_input_buffering();

    if (argc < 2)
    {
        printf("Usage: %s path/to/file", args[0]);
        restore_input_buffering();
        exit(EXIT_FAILURE);
    }

    if (!load_file(args[1]))
    {
        fputs("Fail to open the given file", stdout);
        restore_input_buffering();
        exit(EXIT_FAILURE);
    }
    
    // int debug = (int)args[2][0] - 48;
    running = 1;
    uint16_t instr = 0;
    uint16_t op_code = 0;
    while (running)
    {
        instr = memory[registers[R_PC]++];

        op_code = (instr >> 12) & 0xF;

        // if (debug)
        // {
        //     show_asm(op_code, instr);
        //     getchar();
        // }
        // printf("KBSR->%d\n",memory[0xfe00]);
        // printf("KBDR->%d\n", memory[0xfe02]);

        switch (op_code)
        {
        case OP_BR:
            BR(instr);
            break;
        case OP_ADD:
            ADD(instr);
            break;
        case OP_LD:
            LD(instr);
            break;
        case OP_ST:
            ST(instr);
            break;
        case OP_JSR:
            JSR(instr);
            break;
        case OP_AND:
            AND(instr);
            break;
        case OP_LDR:
            LDR(instr);
            break;
        case OP_STR:
            STR(instr);
            break;
        case OP_NOT:
            NOT(instr);
            break;
        case OP_LDI:
            LDI(instr);
            break;
        case OP_STI:
            STI(instr);
            break;
        case OP_JMP:
            JMP(instr);
            break;
        case OP_LEA:
            LEA(instr);
            break;
        case OP_TRAP:
            TRAP(instr);
            break;
        case OP_RTI:
        default:
            fputs("Exception: BAD OP CODE", stdout);
            restore_input_buffering();
            // HALT();
            abort();
            break;
        }
    }
    restore_input_buffering();
    return 0;
}

int load_file(const char *file_path)
{
    FILE *file = fopen(file_path, "rb");
    if (!file)
        return 0;
    read_file(file);
    fclose(file);
    return 1;
}

void read_file(FILE *file)
{
    uint16_t origin;
    fread(&origin, sizeof(uint16_t), 1, file);
    origin = swap16(origin);
    registers[R_PC] = origin;
    uint16_t *p = memory + origin;
    size_t read = fread(p, sizeof(uint16_t), MAX_MEMORY - origin, file);

    /* Swap the lowest bit with the highest bit, because lc3 uses big-endian */
    while (read-- > 0)
    {
        *p = swap16(*p);
        ++p;
    }
}

uint16_t swap16(uint16_t x)
{
    return (x << 8) | (x >> 8);
}

void show_binary(uint16_t instr)
{
    int bit_count = 0;
    while (bit_count++ < 16)
    {
        printf("%d", (instr & 0x8000) >> 15);
        if (bit_count % 4 == 0)
            putchar(' ');
        instr <<= 1;
    }
}

/* Print out machine codes for debugging */
void show_asm(uint16_t op_code, uint16_t instr)
{
    switch (op_code)
    {
    case OP_BR:
        fputs("br->", stdout);
        show_binary(instr);
        break;
    case OP_ADD:
        fputs("add->", stdout);
        show_binary(instr);
        break;
    case OP_LD:
        fputs("ld->", stdout);
        show_binary(instr);
        break;
    case OP_ST:
        fputs("st->", stdout);
        show_binary(instr);
        break;
    case OP_JSR:
        fputs("jsr->", stdout);
        show_binary(instr);
        break;
    case OP_AND:
        fputs("and->", stdout);
        show_binary(instr);
        break;
    case OP_LDR:
        fputs("ldr->", stdout);
        show_binary(instr);
        break;
    case OP_STR:
        fputs("str->", stdout);
        show_binary(instr);
        break;
    case OP_NOT:
        fputs("not->", stdout);
        show_binary(instr);
        break;
    case OP_LDI:
        fputs("ldi->", stdout);
        show_binary(instr);
        break;
    case OP_STI:
        fputs("sti->", stdout);
        show_binary(instr);
        break;
    case OP_JMP:
        fputs("jmp->", stdout);
        show_binary(instr);
        break;
    case OP_LEA:
        fputs("lea->", stdout);
        show_binary(instr);
        break;
    case OP_TRAP:
        fputs("trap->", stdout);
        show_binary(instr);
        break;
    case OP_RTI:
    default:
        fputs("Exception: BAD OP CODE", stdout);
        show_binary(instr);
        break;
    }
}