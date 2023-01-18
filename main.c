/* 046267 Computer Architecture - Winter 20/21 - HW #4 */

#include <stdio.h>
#include "core_api.h"
#include "sim_api.h"

int main(int argc, char const *argv[]){
	char const *memFname = argv[1];

	if (SIM_MemReset(memFname) != 0) {
		fprintf(stderr, "Failed initializing memory simulator!\n");
	    exit(2);
	}

	int threads = SIM_GetThreadsNum();

    // Allocate register files
    tcontext *blocked = (tcontext*)malloc(threads * sizeof(tcontext));
    tcontext *finegrained = (tcontext*)malloc(threads * sizeof(tcontext));
    
    // Init thread registers
	for(int k=0; k<threads; k++) {
	    for (int i=0; i<REGS_COUNT; i++) {
	        finegrained[k].reg[i] = 0;
	    	blocked[k].reg[i] = 0;
	    }
	}

    // Start blocked MT simulation
	CORE_BlockedMT();
	printf("\n---- Blocked MT Simulation ----\n");
	for(int k=0; k<threads; k++){
		CORE_BlockedMT_CTX(blocked, k);
	    printf("\nRegister file thread id %d:\n", k);
	    for (int i=0; i<REGS_COUNT; ++i)
	        printf("\tR%d = 0x%X", i, blocked[k].reg[i]);
	}
    printf("\nBlocked MT CPI for this program %lf\n", CORE_BlockedMT_CPI());

    // Start finegrained MT simulation
	CORE_FinegrainedMT();
	printf("\n-----Finegrained MT Simulation -----\n");
	for(int k=0; k < SIM_GetThreadsNum(); k++){
		CORE_FinegrainedMT_CTX(finegrained,k);
	    printf("\nRegister file thread id %d:\n",k);
	    for (int i = 0; i < REGS_COUNT; ++i)
	        printf("\tR%d = 0x%X", i, finegrained[k].reg[i]);
	}
	printf("\nFinegrained Multithreading CPI for this program %lf\n\n", CORE_FinegrainedMT_CPI());
	SIM_MemFree();

    // Free register files
    free(blocked);
    free(finegrained);

	return 0;
}


