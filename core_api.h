/* 046267 Computer Architecture - Winter 20/21 - HW #4 */

#ifndef CORE_API_H_
#define CORE_API_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#define REGS_COUNT 8

typedef enum {
	CMD_NOP = 0,
    CMD_ADD,     // dst <- src1 + src2
    CMD_SUB,     // dst <- src1 - src2
    CMD_ADDI,    // dst <- src1 + imm
    CMD_SUBI,    // dst <- src1 - imm
    CMD_LOAD,    // dst <- Mem[src1 + src2]  (src2 may be an immediate)
    CMD_STORE,   // Mem[dst + src2] <- src1  (src2 may be an immediate)
	CMD_HALT,
} cmd_opcode;

typedef struct _inst {
	cmd_opcode opcode;
	int dst_index;
	int src1_index;
	int src2_index_imm;
	bool isSrc2Imm; // if the second argument is immediate
} Instruction;

typedef struct _regs {
	int reg[REGS_COUNT];
} tcontext;


/* Simulates blocked MT and fine-grained MT behavior, respectively */
void CORE_BlockedMT();
void CORE_FinegrainedMT();

/* Get thread register file through the context pointer */
void CORE_BlockedMT_CTX(tcontext context[], int threadid);
void CORE_FinegrainedMT_CTX(tcontext context[], int threadid);

/* Return performance in CPI metric */
double CORE_BlockedMT_CPI();
double CORE_FinegrainedMT_CPI();

#ifdef __cplusplus
}
#endif

#endif /* CORE_API_H_ */
