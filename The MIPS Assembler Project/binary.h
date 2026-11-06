/* binary.h
 *
 * Binary / hexadecimal conversion utilities.
 *
 * BBM234 — Computer Organization
 * Programming Assignment: The MIPS Assembler
 *
 * NOTE: You MUST implement int_to_binary() and binary_to_hex() yourself.
 *       You may NOT use any standard or third-party utility that does
 *       binary or hex conversion for you (no printf("%b"...), no itoa(),
 *       no sprintf("%x"...) for the final hex output, etc.).
 */

#ifndef BINARY_H
#define BINARY_H

/**
 * Convert a (possibly negative) integer to a fixed-width binary string.
 *
 *   value : the integer to convert.
 *   width : number of bits in the output (1..32).
 *   out   : caller-provided buffer of at least (width + 1) chars.
 *           On return, contains exactly `width` characters of '0' / '1'
 *           followed by a terminating '\0'.
 *
 * For negative values, use TWO'S COMPLEMENT in the given width.
 *
 * Examples:
 *   int_to_binary(   5,  5, out) -> "00101"
 *   int_to_binary(  -2,  8, out) -> "11111110"
 *   int_to_binary(  48, 16, out) -> "0000000000110000"
 *   int_to_binary(  -5, 16, out) -> "1111111111111011"
 *   int_to_binary(   2, 26, out) -> "00000000000000000000000010"
 */
void int_to_binary(int value, int width, char *out);

/**
 * Convert a 32-character binary string to an 8-character lowercase hex string.
 *
 *   binary : input string of EXACTLY 32 characters of '0' / '1'.
 *   out    : caller-provided buffer of at least 9 chars.
 *            On return, contains exactly 8 lowercase hex digits followed
 *            by a terminating '\0'. NO "0x" prefix.
 *
 * Example:
 *   binary_to_hex("00000001010010110100100000100000", out) -> "014b4820"
 */
void binary_to_hex(const char *binary, char *out);

#endif /* BINARY_H */
