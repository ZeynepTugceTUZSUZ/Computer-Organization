/* assembler.c
 *
 * Implementation of the MIPS assembler.
 *
 * The functions in the "PROVIDED" sections are complete and you should
 * not normally need to modify them. The four functions in the
 * "TODO" section at the bottom are yours to implement.
 *
 * BBM234 — Computer Organization
 */

#include "assembler.h"
#include "binary.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ====================================================================== */
/*   Internal helpers                                                     */
/* ====================================================================== */

/* Strip trailing '\r', '\n', spaces, and tabs from `s`. */
static void rstrip(char *s)
{
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r' ||
                     s[n - 1] == ' '  || s[n - 1] == '\t')) {
        s[--n] = '\0';
    }
}

/* Bounded copy: copy `src` into `dst` (size `dst_size`), truncating if
 * needed. Always null-terminates. Avoids -Wstringop-truncation from
 * strncpy and -Wformat-truncation from snprintf. */
static void safe_copy(char *dst, size_t dst_size, const char *src)
{
    if (dst_size == 0) return;
    size_t i = 0;
    while (i + 1 < dst_size && src[i] != '\0') {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

/* Copy `raw` into `packed`, dropping any whitespace. Stops at 32 bits. */
static void pack_encoding(const char *raw, char *packed)
{
    int j = 0;
    for (int i = 0; raw[i] != '\0' && j < ENCODING_BITS; i++) {
        if (raw[i] != ' ' && raw[i] != '\t') {
            packed[j++] = raw[i];
        }
    }
    packed[ENCODING_BITS] = '\0';
}

/* Extract operand placeholder characters from a syntax string.
 *
 * Conventions:
 *   - The mnemonic (the very first whitespace-separated token) is skipped.
 *   - Subsequent tokens are separated by spaces, commas, and parentheses.
 *   - For each operand token, the "interesting" character is the first
 *     non-'$' letter, except:
 *         "offset" -> 'i'
 *         "imm"    -> 'i'
 *
 * Examples:
 *   "add $d, $s, $t"     -> ['d', 's', 't']
 *   "lw $t, offset($s)"  -> ['t', 'i', 's']
 *   "slti $t, $s, imm"   -> ['t', 's', 'i']
 *   "j imm"              -> ['i']
 */
static void extract_operands(const char *syntax,
                             char       *chars,
                             int        *count)
{
    *count = 0;
    char buf[MAX_SYNTAX_LEN];
    strncpy(buf, syntax, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *tok = strtok(buf, " ,()\t");
    int   first_token = 1;
    while (tok != NULL && *count < MAX_OPERANDS) {
        if (first_token) {
            first_token = 0;                  /* skip the mnemonic */
        } else {
            const char *p = tok;
            if (*p == '$') p++;

            char c;
            if (strncmp(p, "offset", 6) == 0) {
                c = 'i';
            } else if (strncmp(p, "imm", 3) == 0) {
                c = 'i';
            } else {
                c = *p;
            }
            chars[(*count)++] = c;
        }
        tok = strtok(NULL, " ,()\t");
    }
}

/* Classify an instruction structurally (NOT by mnemonic).
 *
 *   - No 'i' in encoding         -> R-type
 *   - 'i' run length >= 26       -> J-type
 *   - "offset" keyword w/o '('   -> Branch
 *   - otherwise                  -> non-branch I-type
 */
static InstructionType classify(const char *raw_syntax, const char *encoding)
{
    int has_i  = 0;
    int n_i    = 0;
    for (int i = 0; encoding[i] != '\0'; i++) {
        if (encoding[i] == 'i') { has_i = 1; n_i++; }
    }
    if (!has_i)        return TYPE_R;
    if (n_i >= 26)     return TYPE_J;

    int has_offset = (strstr(raw_syntax, "offset") != NULL);
    int has_paren  = (strchr(raw_syntax, '(')      != NULL);
    if (has_offset && !has_paren) return TYPE_BRANCH;

    return TYPE_I;
}

/* ====================================================================== */
/*   PROVIDED — load_registers                                            */
/* ====================================================================== */
int load_registers(const char *path, RegisterTable *out)
{
    FILE *fp = fopen(path, "r");
    if (fp == NULL) return -1;

    out->count = 0;

    char line[128];
    while (fgets(line, sizeof(line), fp) != NULL) {
        rstrip(line);
        if (line[0] == '\0')                continue;
        if (out->count >= MAX_REGISTERS)    break;

        char *tab = strchr(line, '\t');
        if (tab == NULL)                    continue;
        *tab = '\0';

        Register *r = &out->entries[out->count];
        r->number = atoi(line);
        strncpy(r->name, tab + 1, MAX_NAME_LEN - 1);
        r->name[MAX_NAME_LEN - 1] = '\0';
        out->count++;
    }
    fclose(fp);
    return 0;
}

/* ====================================================================== */
/*   PROVIDED — load_instructions                                         */
/* ====================================================================== */
int load_instructions(const char *path, InstructionTable *out)
{
    FILE *fp = fopen(path, "r");
    if (fp == NULL) return -1;

    out->count = 0;

    char line[256];
    while (fgets(line, sizeof(line), fp) != NULL) {
        rstrip(line);
        if (line[0] == '\0')                  continue;
        if (out->count >= MAX_INSTRUCTIONS)   break;

        char *tab = strchr(line, '\t');
        if (tab == NULL)                      continue;
        *tab = '\0';

        InstructionDef *def = &out->entries[out->count];

        /* full syntax (e.g. "add $d, $s, $t") */
        safe_copy(def->syntax, sizeof(def->syntax), line);

        /* mnemonic = first whitespace-delimited token */
        char syntax_copy[MAX_SYNTAX_LEN];
        safe_copy(syntax_copy, sizeof(syntax_copy), line);
        char *first_space = strpbrk(syntax_copy, " \t");
        if (first_space != NULL) *first_space = '\0';
        safe_copy(def->mnemonic, sizeof(def->mnemonic), syntax_copy);

        /* packed encoding (no spaces) */
        pack_encoding(tab + 1, def->encoding);

        /* placeholder characters in operand order */
        extract_operands(line, def->operand_chars, &def->num_operands);

        /* type classification */
        def->type = classify(line, def->encoding);

        out->count++;
    }
    fclose(fp);
    return 0;
}

/* ====================================================================== */
/*   PROVIDED — lookups                                                   */
/* ====================================================================== */
const InstructionDef *find_instruction(const InstructionTable *t,
                                       const char             *mnemonic)
{
    for (int i = 0; i < t->count; i++) {
        if (strcmp(t->entries[i].mnemonic, mnemonic) == 0) {
            return &t->entries[i];
        }
    }
    return NULL;
}

int find_register(const RegisterTable *t, const char *name)
{
    for (int i = 0; i < t->count; i++) {
        if (strcmp(t->entries[i].name, name) == 0) {
            return t->entries[i].number;
        }
    }
    return -1;
}

/* ====================================================================== */
/*   PROVIDED — substitute_field                                          */
/* ====================================================================== */
void substitute_field(char *binary_template, char placeholder, int value)
{
    /* Find the first run of `placeholder` and replace it with the binary
     * representation of `value` at that exact width. */
    char *p = strchr(binary_template, placeholder);
    if (p == NULL) return;

    int width = 0;
    while (p[width] == placeholder) width++;

    char buf[ENCODING_BITS + 1];
    int_to_binary(value, width, buf);
    memcpy(p, buf, (size_t)width);
}

/* ====================================================================== */
/*   PROVIDED — assemble_line                                             */
/* ====================================================================== */
int assemble_line(const char             *line,
                  int                     pc,
                  const RegisterTable    *regs,
                  const InstructionTable *instrs,
                  char                   *binary_out,
                  char                   *hex_out)
{
    char buf[256];
    strncpy(buf, line, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    /* Treat ',' '(' ')' as whitespace for tokenisation. */
    for (int i = 0; buf[i] != '\0'; i++) {
        if (buf[i] == ',' || buf[i] == '(' || buf[i] == ')') {
            buf[i] = ' ';
        }
    }

    char *tok = strtok(buf, " \t");
    if (tok == NULL) return -1;

    const InstructionDef *def = find_instruction(instrs, tok);
    if (def == NULL) return -1;

    int operands[MAX_OPERANDS];
    int num_operands = 0;

    while ((tok = strtok(NULL, " \t")) != NULL && num_operands < MAX_OPERANDS) {
        if (tok[0] == '$') {
            int rn = find_register(regs, tok);
            if (rn < 0) return -1;
            operands[num_operands++] = rn;
        } else {
            operands[num_operands++] = atoi(tok);
        }
    }

    if (num_operands != def->num_operands) return -1;

    int rc;
    switch (def->type) {
        case TYPE_R:      rc = encode_r_type(def, operands, pc, binary_out); break;
        case TYPE_I:      rc = encode_i_type(def, operands, pc, binary_out); break;
        case TYPE_BRANCH: rc = encode_branch(def, operands, pc, binary_out); break;
        case TYPE_J:      rc = encode_j_type(def, operands, pc, binary_out); break;
        default:          return -1;
    }
    if (rc != 0) return -1;

    binary_to_hex(binary_out, hex_out);
    return 0;
}

/* ====================================================================== */
/* ======================================================================
 *
 *                        TODO SECTION — STUDENT WORK
 *
 *   You must implement the four encoder functions below. Each one is
 *   independently graded:
 *
 *      encode_r_type ......... 30%   (R-type)
 *      encode_i_type ......... 20%   (I-type, non-branch)
 *      encode_branch ......... 30%   (I-type branches)
 *      encode_j_type ......... 20%   (J-type)
 *
 *   You may (and should) make use of the substitute_field() helper.
 *   Do NOT branch on the instruction mnemonic anywhere in your code.
 *
 * ======================================================================
 * ====================================================================== */

 

/**
 * TODO [R-TYPE — 30%]
 * --------------------------------------------------------------------
 *  Encode an R-type instruction (no immediate field).
 *
 *  - Copy the encoding template (def->encoding) into binary_out.
 *  - For each operand i in 0 .. def->num_operands - 1:
 *        placeholder = def->operand_chars[i]   (one of 'd', 's', 't')
 *        value       = operands[i]             (a register number)
 *    Substitute that value into all positions of the template marked
 *    by `placeholder`. The width is determined by the encoding —
 *    substitute_field() handles this for you.
 *  - The pc parameter is unused for R-type; ignore it.
 *  - Return 0 on success.
 *
 *  After this function returns, binary_out must contain exactly
 *  32 chars of '0' / '1', followed by '\0'.
 */
int encode_r_type(const InstructionDef *def,
                  const int            *operands,
                  int                   pc,
                  char                 *binary_out)
{
    (void)pc;                       /* silence "unused parameter" warning   */

    /* TODO: implement */
    strcpy(binary_out, def->encoding);
    for (int i = 0; i < def->num_operands; i++) {
        char placeholder = def->operand_chars[i];
        int value = operands[i];
        substitute_field(binary_out, placeholder, value);
    }

    return 0;
}

/**
 * TODO [I-TYPE non-branch — 20%]
 * --------------------------------------------------------------------
 *  Encode a non-branch I-type instruction (lw, sw, addi, slti, etc.).
 *
 *  - The pattern is the same as encode_r_type, except that one of the
 *    operands is an immediate (placeholder 'i'). The immediate is
 *    encoded DIRECTLY (no transformation) at whatever bit width the
 *    encoding template specifies.
 *  - Negative immediates must be encoded as two's complement.
 *  - The pc parameter is unused; ignore it.
 *  - Return 0 on success.
 */
int encode_i_type(const InstructionDef *def,
                  const int            *operands,
                  int                   pc,
                  char                 *binary_out)
{
    (void)pc;

    /* TODO: implement */
    strcpy(binary_out, def->encoding);
    for (int i = 0; i < def->num_operands; i++) {
        char placeholder = def->operand_chars[i];
        int value = operands[i];
        substitute_field(binary_out, placeholder, value);
    }

    return 0;
}

/**
 * TODO [BRANCH — 30%]
 * --------------------------------------------------------------------
 *  Encode a branch instruction (beq, bne, ...).
 *
 *  Calling convention for branch operands in this assignment:
 *    - The branch's third operand is the TARGET BYTE ADDRESS of the
 *      branch (not a label, not a raw offset).
 *    - The first instruction in program.txt is at byte address 0,
 *      the second at 4, the third at 8, and so on.
 *
 *  Encoding:
 *    - Compute the PC-relative offset (in *instructions*):
 *
 *          offset = (target - pc - 4) / 4
 *
 *    - Place this offset, sign-extended into a 16-bit two's-complement
 *      field, into the 'i' positions of the encoding template.
 *    - Register operands ($s, $t) are substituted exactly as in
 *      encode_i_type.
 *    - Return 0 on success.
 *
 *  Hint: int_to_binary() should already produce the correct two's-
 *        complement representation for negative values. You only need
 *        to compute the right offset and pass it in.
 */
int encode_branch(const InstructionDef *def,
                  const int            *operands,
                  int                   pc,
                  char                 *binary_out)
{
    /* TODO: implement */
   strcpy(binary_out, def->encoding);

    for (int i = 0; i < def->num_operands; i++) {
        char placeholder = def->operand_chars[i];
        int value;

        if (placeholder == 'i') {
            int target_address = operands[i];
            value = (target_address - (pc + 4)) / 4;

        } else {value = operands[i];}
        
        substitute_field(binary_out, placeholder, value);
    }

    return 0;
}

/**
 * TODO [J-TYPE — 20%]
 * --------------------------------------------------------------------
 *  Encode a J-type instruction (j, jal, ...).
 *
 *  Calling convention for the J-type operand in this assignment:
 *    - The single operand is the TARGET BYTE ADDRESS of the jump.
 *
 *  Encoding:
 *    - The encoded value is target / 4   (i.e. target shifted right
 *      by 2 bits; the bottom two bits of an aligned MIPS instruction
 *      address are always zero, and are not stored in the instruction).
 *    - Place this value into the 26-bit 'i' field of the encoding.
 *    - Return 0 on success.
 *
 *  The pc parameter is provided for completeness but is not needed
 *  for the simple form of `j` used in this assignment.
 */
int encode_j_type(const InstructionDef *def,
                  const int            *operands,
                  int                   pc,
                  char                 *binary_out)
{
    (void)pc;

    /* TODO: implement */
    strcpy(binary_out, def->encoding);
    for (int i = 0; i < def->num_operands; i++) {
        char placeholder = def->operand_chars[i];
        int value;

        if (placeholder == 'i') {value = operands[i] / 4;}
        else {value = operands[i];}

        substitute_field(binary_out, placeholder, value);
    }

    return 0;
}
