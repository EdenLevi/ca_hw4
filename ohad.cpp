/* 046267 Computer Architecture - Winter 20/21 - HW #4 */

#include "core_api.h"
#include "sim_api.h"


#include <stdio.h>
#include <iostream>
#include "vector"
#include <algorithm>


class Thread {
public:
    int threadNumber;
    int currentInstruction;
    int waitingTime;
    bool halt;
};

int nextThreadAvailable(int thread, std::vector<Thread>* currentThreads);
void decreaseWaitingTime(std::vector<Thread>* currentThreads, int time);
bool isFinished(std::vector<Thread>* currentThreads);


std::vector<_regs> *regsThreadsBlockedGlobal ;
std::vector<_regs> *regsThreadsFineGlobal;
int cyclesBlockedGlobal = 0;
int operationsBlockedGlobal = 0;
int cyclesFinedGlobal = 0;
int operationsFinedGlobal = 0;


void CORE_BlockedMT() {
    _inst *dst = new Instruction();
    int threads = SIM_GetThreadsNum();
    std::vector<Thread> *currentThreads = new std::vector<Thread>(threads);
    int thread = 0;
    int old_thread = 0;
    int immIndex = 0;
    int operationsBlocked =0;
    int cyclesBlocked =0;

    int32_t *result = new int32_t();

    std::vector<_regs> *regsThreadsBlocked = new std::vector<_regs>(SIM_GetThreadsNum());

// Initialization
    for (int i = 0; i < threads; i++) {
        (*currentThreads)[i].threadNumber = i;
        (*currentThreads)[i].currentInstruction = 0;
        (*currentThreads)[i].waitingTime = 0;
        (*currentThreads)[i].halt = false;
    }

    while (!isFinished(currentThreads)) {
        if ((*currentThreads)[thread].waitingTime == 0) {
            operationsBlocked++;
            SIM_MemInstRead((*currentThreads)[thread].currentInstruction, dst, (*currentThreads)[thread].threadNumber);
            decreaseWaitingTime(currentThreads,SIM_GetSwitchCycles());

            if (dst->opcode == CMD_HALT) {
                (*currentThreads)[thread].halt = true;
                (*currentThreads)[thread].currentInstruction = -1;
                cyclesBlocked += SIM_GetSwitchCycles();
                old_thread = thread;
                thread = nextThreadAvailable(thread, currentThreads);
//decreaseWaitingTime(currentThreads,SIM_GetSwitchCycles());

            } else if (dst->opcode == CMD_LOAD) {
                (*currentThreads)[thread].waitingTime = SIM_GetLoadLat()+1;
                SIM_MemDataRead(dst->src1_index + dst->src2_index_imm, result);
                (*regsThreadsBlocked)[thread].reg[dst->dst_index] = *result;
                old_thread = thread;
                thread = nextThreadAvailable(thread, currentThreads);
                if (old_thread != thread) {
                    cyclesBlocked += SIM_GetSwitchCycles();
//decreaseWaitingTime(currentThreads,SIM_GetSwitchCycles());
                }
            } else if (dst->opcode == CMD_STORE) {
                (*currentThreads)[thread].waitingTime = SIM_GetStoreLat()+1;
                SIM_MemDataWrite(dst->dst_index + dst->src2_index_imm, (*regsThreadsBlocked)[thread].reg[dst->src1_index]);
                old_thread = thread;
                thread = nextThreadAvailable(thread, currentThreads);
                if (old_thread != thread) {
                    cyclesBlocked += SIM_GetSwitchCycles();
//decreaseWaitingTime(currentThreads,SIM_GetSwitchCycles());
                }
            } else if (dst->opcode == CMD_ADD) {
                (*currentThreads)[thread].waitingTime = 1;
                old_thread = thread;
                (*regsThreadsBlocked)[thread].reg[dst->dst_index] = (*regsThreadsBlocked)[thread].reg[dst->src1_index]
                                                                    + (*regsThreadsBlocked)[thread].reg[dst->src2_index_imm];
            } else if (dst->opcode == CMD_ADDI) {
                (*currentThreads)[thread].waitingTime = 1;
                old_thread = thread;
                if (!dst->isSrc2Imm) {
                    immIndex = (*regsThreadsBlocked)[thread].reg[dst->src2_index_imm];
                } else {
                    immIndex = dst->src2_index_imm;
                }
                (*regsThreadsBlocked)[thread].reg[dst->dst_index] = (*regsThreadsBlocked)[thread].reg[dst->src1_index] + immIndex;
            } else if (dst->opcode == CMD_SUB) {
                (*currentThreads)[thread].waitingTime = 1;
                old_thread = thread;
                (*regsThreadsBlocked)[thread].reg[dst->dst_index] = (*regsThreadsBlocked)[thread].reg[dst->src1_index]
                                                                    -(*regsThreadsBlocked)[thread].reg[dst->src2_index_imm];
            } else if (dst->opcode == CMD_SUBI) {
                (*currentThreads)[thread].waitingTime = 1;
                old_thread = thread;
                if (!dst->isSrc2Imm) {
                    immIndex = (*regsThreadsBlocked)[thread].reg[dst->src2_index_imm];
                } else {
                    immIndex = dst->src2_index_imm;
                }
                (*regsThreadsBlocked)[thread].reg[dst->dst_index] =
                        (*regsThreadsBlocked)[thread].reg[dst->src1_index] - immIndex;
            }
            decreaseWaitingTime(currentThreads,1);
            (*currentThreads)[old_thread].currentInstruction++;
        } else {
            decreaseWaitingTime(currentThreads,1);
        }
        cyclesBlocked++;

    }

    regsThreadsBlockedGlobal = regsThreadsBlocked;
    cyclesBlockedGlobal= cyclesBlocked-SIM_GetSwitchCycles();
    operationsBlockedGlobal = operationsBlocked;
    delete dst;
    delete result;
    delete currentThreads;
    //delete regsThreadsBlocked;

}


void decreaseWaitingTime(std::vector<Thread>* currentThreads, int time) {
    for (int i = 0; i < (int)currentThreads->size(); i++) {
        if ((*currentThreads)[i].waitingTime - time > 0) {
            (*currentThreads)[i].waitingTime -= time;
        }
        else {
            (*currentThreads)[i].waitingTime = 0;
        }
    }
}


int nextThreadRR(int index, int size) {
    if(index == size - 1)
        return 0;
    else
        return index + 1;
}

int nextThreadAvailable(int thread, std::vector<Thread>* currentThreads) {
    int threadWait = (*currentThreads)[thread].waitingTime;
    int numberOfThreads = 0;
    int fastestThread = thread;
    int fastestThreadTime = threadWait;
    int nextThreadWait = 0;

    int nextThread = nextThreadRR(thread,currentThreads->size());
    if((*currentThreads)[thread].halt)
    {
        fastestThreadTime = 999999;
    }
    while (numberOfThreads != (int)currentThreads->size()) {
        nextThreadWait = (*currentThreads)[nextThread].waitingTime;
        if (nextThreadWait < fastestThreadTime && !(*currentThreads)[nextThread].halt) {
            fastestThreadTime = nextThreadWait;
            fastestThread = nextThread;
        }
        numberOfThreads++;
        nextThread = nextThreadRR(nextThread,(int)currentThreads->size());
    }
    return fastestThread;
}

bool isFinished(std::vector<Thread>* currentThreads) {
    for (int i = 0; i < (int)currentThreads->size(); i++) {
        if(!(*currentThreads)[i].halt) {
            return false;
        }
    }
    return true;
}
int nextThreadAvailableFine(int thread, std::vector<Thread>* currentThreads) {
    int nextThread = nextThreadRR(thread,currentThreads->size());
    int threadWait = (*currentThreads)[nextThread].waitingTime;
    int numberOfThreads = 0;
    int fastestThread = nextThread;
    int fastestThreadTime = threadWait;
    if (fastestThreadTime <= 1 && !(*currentThreads)[nextThread].halt)
    {
        return fastestThread;
    }
    int nextThreadWait = 0;
    int old_thread = nextThread;
    nextThread = nextThreadRR(nextThread,currentThreads->size());


    if((*currentThreads)[old_thread].halt)
    {
        fastestThreadTime = 999999;
    }
    while (numberOfThreads != (int)currentThreads->size()) {
        nextThreadWait = (*currentThreads)[nextThread].waitingTime;
        if (nextThreadWait <= 1 && !(*currentThreads)[nextThread].halt)
        {
            return nextThread;
        }
        if (nextThreadWait < fastestThreadTime && !(*currentThreads)[nextThread].halt) {
            fastestThreadTime = nextThreadWait;
            fastestThread = nextThread;
        }
        numberOfThreads++;
        nextThread = nextThreadRR(nextThread,currentThreads->size());
    }
    return fastestThread;
}

void CORE_FinegrainedMT() {
    _inst *dst = new Instruction();
    int threads = SIM_GetThreadsNum();
    std::vector<Thread> *currentThreads = new std::vector<Thread>(threads);
    int thread = 0;
    int old_thread = 0;
    int immIndex = 0;
    int operationsFine =0;
    int cyclesFine =0;


    int32_t *result = new int32_t();

    std::vector<_regs> *regsThreadsFine = new std::vector<_regs>(SIM_GetThreadsNum());

// Initialization
    for (int i = 0; i < threads; i++) {
        (*currentThreads)[i].threadNumber = i;
        (*currentThreads)[i].currentInstruction = 0;
        (*currentThreads)[i].waitingTime = 0;
        (*currentThreads)[i].halt = false;

    }

    while (!isFinished(currentThreads)) {

        if ((*currentThreads)[thread].waitingTime == 0) {
            operationsFine++;
            SIM_MemInstRead((*currentThreads)[thread].currentInstruction, dst, (*currentThreads)[thread].threadNumber);

            if (dst->opcode == CMD_HALT) {
                (*currentThreads)[thread].halt = true;
                (*currentThreads)[thread].currentInstruction = -1;
                old_thread = thread;
                thread = nextThreadAvailableFine(thread, currentThreads);

            } else if (dst->opcode == CMD_LOAD) {
                (*currentThreads)[thread].waitingTime = SIM_GetLoadLat()+1;
                SIM_MemDataRead(dst->src1_index + dst->src2_index_imm, result);
                (*regsThreadsFine)[thread].reg[dst->dst_index] = *result;
                old_thread = thread;
                thread = nextThreadAvailableFine(thread, currentThreads);
            } else if (dst->opcode == CMD_STORE) {
                (*currentThreads)[thread].waitingTime = SIM_GetStoreLat()+1;
                SIM_MemDataWrite(dst->dst_index + dst->src2_index_imm,                 (*regsThreadsFine)[thread].reg[dst->src1_index]
                );
                old_thread = thread;
                thread = nextThreadAvailableFine(thread, currentThreads);

            } else if (dst->opcode == CMD_ADD) {
                (*currentThreads)[thread].waitingTime = 1;
                old_thread = thread;
                (*regsThreadsFine)[thread].reg[dst->dst_index] = (*regsThreadsFine)[thread].reg[dst->src1_index]
                                                                 +
                                                                 (*regsThreadsFine)[thread].reg[dst->src2_index_imm];
                thread = nextThreadAvailableFine(thread, currentThreads);
            } else if (dst->opcode == CMD_ADDI) {
                (*currentThreads)[thread].waitingTime = 1;
                old_thread = thread;
                if (!dst->isSrc2Imm) {
                    immIndex = (*regsThreadsFine)[thread].reg[dst->src2_index_imm];
                } else {
                    immIndex = dst->src2_index_imm;

                }
                (*regsThreadsFine)[thread].reg[dst->dst_index] =
                        (*regsThreadsFine)[thread].reg[dst->src1_index] + immIndex;
                old_thread = thread;
                thread = nextThreadAvailableFine(thread, currentThreads);

            } else if (dst->opcode == CMD_SUB) {
                (*currentThreads)[thread].waitingTime = 1;
                old_thread = thread;
                (*regsThreadsFine)[thread].reg[dst->dst_index] = (*regsThreadsFine)[thread].reg[dst->src1_index]
                                                                 -
                                                                 (*regsThreadsFine)[thread].reg[dst->src2_index_imm];
                thread = nextThreadAvailableFine(thread, currentThreads);

            } else if (dst->opcode == CMD_SUBI) {
                (*currentThreads)[thread].waitingTime = 1;
                old_thread = thread;
                if (!dst->isSrc2Imm) {
                    immIndex = (*regsThreadsFine)[thread].reg[dst->src2_index_imm];
                } else {
                    immIndex = dst->src2_index_imm;
                }
                (*regsThreadsFine)[thread].reg[dst->dst_index] =
                        (*regsThreadsFine)[thread].reg[dst->src1_index] - immIndex;
                thread = nextThreadAvailableFine(thread, currentThreads);

            }
            decreaseWaitingTime(currentThreads,1);
            (*currentThreads)[old_thread].currentInstruction++;
        } else {
            decreaseWaitingTime(currentThreads,1);
        }
        cyclesFine++;

    }

    regsThreadsFineGlobal = regsThreadsFine;
    cyclesFinedGlobal= cyclesFine;
    operationsFinedGlobal = operationsFine;
    delete dst;
    delete result;
    delete currentThreads;
    //delete regsThreadsFine;

}




double CORE_BlockedMT_CPI(){
    delete regsThreadsBlockedGlobal;
    return (double)cyclesBlockedGlobal/(double)operationsBlockedGlobal;
}

double CORE_FinegrainedMT_CPI(){
    delete regsThreadsFineGlobal;
    std::cout << "FinegrainedMT_CPI = " << cyclesFinedGlobal << " / " << operationsFinedGlobal << std::endl;
    return (double)cyclesFinedGlobal/(double)operationsFinedGlobal;
}

void CORE_BlockedMT_CTX(tcontext* context, int threadid) {
    for (int i = 0; i < SIM_GetThreadsNum(); i++) {
        for (int j = 0; j< REGS_COUNT; j++) {
            context[i].reg[j] = (*regsThreadsBlockedGlobal)[i].reg[j];
        }
    }
// if (threadid == SIM_GetThreadsNum() -1)
// {
// delete regsThreadsBlockedGlobal;
// }
}

void CORE_FinegrainedMT_CTX(tcontext* context, int threadid) {
    for (int i = 0; i < SIM_GetThreadsNum(); i++) {
        for (int j = 0; j< REGS_COUNT; j++) {
            context[i].reg[j] = (*regsThreadsFineGlobal)[i].reg[j];
        }
    }
// if (threadid == SIM_GetThreadsNum() -1)
// {
// delete regsThreadsFineGlobal;
// }
}
//updated