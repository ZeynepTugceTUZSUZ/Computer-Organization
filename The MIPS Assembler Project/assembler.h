/* assembler.h
 *
 * Public interface for the MIPS assembler.
 *
 * BBM234 — Computer Organization
 * Programming Assignment: The MIPS Assembler
 */

#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#define MAX_REGISTERS    64
#define MAX_INSTRUCTIONS 64
#define MAX_OPERANDS      8
#define MAX_NAME_LEN     16
#define MAX_SYNTAX_LEN   80
#define ENCODING_BITS    32

/* ---------------------------------------------------------------------- */
/*   Instruction type — used for partial-credit dispatch                  */
/* ---------------------------------------------------------------------- */
typedef enum {
    TYPE_R,         /* R-type:  no immediate field at all                 */
    TYPE_I,         /* I-type non-branch: loads, stores, immediate ALU    */
    TYPE_BRANCH,    /* I-type branches:   PC-relative offset              */
    TYPE_J,         /* J-type:            26-bit shifted target address   */
    TYPE_UNKNOWN
} InstructionType;

/* ---------------------------------------------------------------------- */
/*   Loaded register table                                                */
/* ---------------------------------------------------------------------- */
typedef struct {
    char name[MAX_NAME_LEN];   /* e.g. "$t0"                               */
    int  number;               /* e.g. 8                                  */
} Register;

typedef struct {
    Register entries[MAX_REGISTERS];
    int      count;
} RegisterTable;

/* ---------------------------------------------------------------------- */
/*   Loaded instruction definition                                        */
/* ---------------------------------------------------------------------- */
typedef struct {
    char            mnemonic[MAX_NAME_LEN];      /* "add", "lw", ...      */
    char            syntax[MAX_SYNTAX_LEN];      /* "add $d, $s, $t"      */
    char            encoding[ENCODING_BITS + 1]; /* 32 chars, no spaces   */
    char            operand_chars[MAX_OPERANDS]; /* placeholders in order */
    int             num_operands;
    InstructionType type;
} InstructionDef;

typedef struct {
    InstructionDef entries[MAX_INSTRUCTIONS];
    int            count;
} InstructionTable;

/* ====================================================================== */
/*   Loaders (PROVIDED)                                                   */
/* ====================================================================== */
int load_registers   (const char *path, RegisterTable    *out);
int load_instructions(const char *path, InstructionTable *out);

/* ====================================================================== */
/*   Lookup helpers (PROVIDED)                                            */
/* ====================================================================== */
const InstructionDef *find_instruction(const InstructionTable *t,
                                       const char             *mnemonic);
int                   find_register   (const RegisterTable    *t,
                                       const char             *name);

/* ====================================================================== */
/*   Substitution helper (PROVIDED)                                       */
/*                                                                        */
/*   Finds the first run of `placeholder` characters in `binary_template` */
/*   and overwrites that run (in place) with the binary representation    */
/*   of `value`, using a width equal to the run length.                   */
/*                                                                        */
/*   Internally calls int_to_binary().                                    */
/* ====================================================================== */
void substitute_field(char *binary_template, char placeholder, int value);

/* ====================================================================== */
/*   Top-level dispatch (PROVIDED)                                        */
/*                                                                        */
/*     line       : one line of assembly, e.g. "add $t1, $t2, $t3"        */
/*     pc         : byte address of this instruction (0, 4, 8, 12, ...)   */
/*     binary_out : 33-char buffer (32 bits + '\0')                       */
/*     hex_out    :  9-char buffer ( 8 hex digits + '\0', no "0x")        */
/*                                                                        */
/*   Returns 0 on success, -1 on failure.                                 */
/* ====================================================================== */
int assemble_line(const char             *line,
                  int                     pc,
                  const RegisterTable    *regs,
                  const InstructionTable *instrs,
                  char                   *binary_out,
                  char                   *hex_out);

/* ====================================================================== */
/*   Type-specific encoders (TODO — students implement these)             */
/* ====================================================================== */

/* TODO [R-TYPE — 30%] */
int encode_r_type(const InstructionDef *def,
                  const int            *operands,
                  int                   pc,
                  char                 *binary_out);

/* TODO [I-TYPE non-branch — 20%] */
int encode_i_type(const InstructionDef *def,
                  const int            *operands,
                  int                   pc,
                  char                 *binary_out);

/* TODO [BRANCH — 30%] */
int encode_branch(const InstructionDef *def,
                  const int            *operands,
                  int                   pc,
                  char                 *binary_out);

/* TODO [J-TYPE — 20%] */
int encode_j_type(const InstructionDef *def,
                  const int            *operands,
                  int                   pc,
                  char                 *binary_out);

#endif /* ASSEMBLER_H */
