/* binary.c
 *
 * TODO — Implement the two functions below.
 *
 * These two functions are PREREQUISITES for receiving any partial credit:
 * if either is broken, every test case in every category will fail because
 * encoded fields and the hex output line both depend on them.
 *
 * REMINDER: no library / built-in conversion is allowed. Build the strings
 *           yourself, bit by bit / nibble by nibble.
 */

#include "binary.h"

#include <stddef.h>

/* ====================================================================== */
/*   TODO [PREREQUISITE]:  int -> binary string                           */
/* ====================================================================== */
void int_to_binary(int value, int width, char *out)
{
    /* TODO:
     *   - Produce exactly `width` characters of '0' / '1' in out[].
     *   - For negative values, use two's complement: take the bottom
     *     `width` bits of the value's two's-complement representation.
     *   - Null-terminate (out[width] = '\0').
     *
     * Hints:
     *   - Iterate from bit (width - 1) down to bit 0; each bit i can be
     *     extracted as ((unsigned)value >> i) & 1u .
     *   - Don't forget the null terminator at the end.
     */

    /* Stub so the program links — replace this with your implementation. */
   unsigned int val = (unsigned int)value;
    
    for (int i = 0; i < width; i++) {
        if (val & (1u << (width - 1 - i))) {
            out[i] = '1';
        } 
        else {out[i] = '0';}
    }
    out[width] = '\0';
}

/* ====================================================================== */
/*   TODO [PREREQUISITE]:  32-char binary string -> 8-char lowercase hex  */
/* ====================================================================== */
void binary_to_hex(const char *binary, char *out)
{
    /* TODO:
     *   - Read `binary` as 32 chars of '0' / '1' (no '0x' prefix).
     *   - Group every 4 consecutive bits into one hex digit.
     *   - Use lowercase letters for digits a..f.
     *   - Write 8 hex characters into out[] and null-terminate.
     *
     * Hints:
     *   - For each group of 4 bits, build a value 0..15 by shifting
     *     and ORing; then map to a character: digits 0..9 -> '0'..'9',
     *     digits 10..15 -> 'a'..'f'.
     */

    /* Stub so the program links — replace this with your implementation. */
    int group = 0;
    while (group < 8) {
        int val = 0;
        for (int b = 0; b < 4; b++) {
            val = (val << 1) | (binary[group * 4 + b] - '0');
        }
        out[group] = (val < 10) ? (char)('0' + val) : (char)('a' + (val - 10));
        
        group++;
    }
    out[8] = '\0';
}
