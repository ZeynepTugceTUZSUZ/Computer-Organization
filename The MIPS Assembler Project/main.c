/* main.c
 *
 * Program entry point for the MIPS assembler.
 *
 * BBM234 — Computer Organization
 * Programming Assignment: The MIPS Assembler
 *
 * This file is provided to you complete. You should NOT need to modify
 * it; the work is in assembler.c and binary.c.
 *
 * Usage:
 *     ./assembler registers.txt instructions.txt program.txt
 *
 * For each non-empty line in program.txt, two lines are written to stdout:
 *     1. the 32-bit binary encoding (32 chars of '0' / '1', no spaces)
 *     2. the 8-digit lowercase hexadecimal encoding, prefixed with "0x"
 */

#include "assembler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LEN 256

int main(int argc, char *argv[])
{
    if (argc != 4) {
        fprintf(stderr,
                "Usage: %s registers.txt instructions.txt program.txt\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    RegisterTable    regs;
    InstructionTable instrs;

    if (load_registers(argv[1], &regs) != 0) {
        fprintf(stderr, "Failed to load registers from %s\n", argv[1]);
        return EXIT_FAILURE;
    }
    if (load_instructions(argv[2], &instrs) != 0) {
        fprintf(stderr, "Failed to load instructions from %s\n", argv[2]);
        return EXIT_FAILURE;
    }

    FILE *fp = fopen(argv[3], "r");
    if (fp == NULL) {
        fprintf(stderr, "Failed to open %s\n", argv[3]);
        return EXIT_FAILURE;
    }

    char line[MAX_LINE_LEN];
    int  pc = 0;        /* first instruction sits at byte address 0 */

    while (fgets(line, sizeof(line), fp) != NULL) {
        /* strip trailing CR / LF */
        line[strcspn(line, "\r\n")] = '\0';
        if (line[0] == '\0') continue;

        /* Pre-fill output buffers with all-zeros so that, if a TODO
         * encoder is unimplemented (or fails), we still emit a
         * line-aligned (but obviously wrong) result instead of nothing. */
        char binary[ENCODING_BITS + 1];
        char hex[9];
        memset(binary, '0', ENCODING_BITS); binary[ENCODING_BITS] = '\0';
        memset(hex,    '0', 8);             hex[8]                = '\0';

        (void)assemble_line(line, pc, &regs, &instrs, binary, hex);

        printf("%s\n0x%s\n", binary, hex);

        pc += 4;        /* every instruction is 4 bytes */
    }

    fclose(fp);
    return EXIT_SUCCESS;
}
