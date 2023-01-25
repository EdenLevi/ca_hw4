/* 046267 Computer Architecture - Winter 20/21 - HW #4 */

#include "core_api.h"
#include "sim_api.h"
#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class thread {
public:
    bool finished;
    tcontext *registers;
    int currInst;
    int idleTimer;

    thread() : finished(false), currInst(0), idleTimer(0) {

        registers = new tcontext();

        for (int i = 0; i < REGS_COUNT; i++) {
            registers->reg[i] = 0;
        }
    }

    ~thread() {
        delete registers;
    }
};

class MT_core {
public:
    int loadLatency;
    int storeLatency;
    int threadsSize;
    thread *threads;
    int cycles;
    int instructions; // CPI will be calculated based on this

    MT_core(int threadsSize) : threadsSize(threadsSize), cycles(0), instructions(0) {
        loadLatency = SIM_GetLoadLat();
        storeLatency = SIM_GetStoreLat();

        threads = new thread[threadsSize];
    }

    virtual ~MT_core() {}

    virtual void load() = 0;

    virtual void store() = 0;

    virtual void add() = 0;

    virtual void addi() = 0;

    virtual void sub() = 0;

    virtual void subi() = 0;

    virtual void contextSwitch() = 0;

};

class BlockCore : virtual public MT_core {
public:
    int switchOverhead;

    BlockCore(int threadsSize) : MT_core(threadsSize) {
        switchOverhead = SIM_GetSwitchCycles(); // relevant only for BLOCK
    }

    ~BlockCore() {
        delete[] threads;
    }

    virtual void load() {

    }

    virtual void store() {

    }

    virtual void add() {

    }

    virtual void addi() {

    }

    virtual void sub() {

    }

    virtual void subi() {

    }

    virtual void contextSwitch() {

    }
};

class FineGrainedCore : virtual public MT_core {
public:
    FineGrainedCore(int threadsSize) : MT_core(threadsSize) {}

    ~FineGrainedCore() {
        delete[] threads;
    }

    void load() {

    }

    virtual void store() {

    }

    virtual void add() {

    }

    virtual void addi() {

    }

    virtual void sub() {

    }

    virtual void subi() {

    }

    virtual void contextSwitch() {

    }
};

BlockCore *block;
FineGrainedCore *fineGrained;
Instruction inst;

// run a simulation of BlockCore
void CORE_BlockedMT() {

    block = new BlockCore(SIM_GetThreadsNum());

    int SIM_MemDataRead_ReturnValue, currentThread = 0, previousThread = 0, nextThread = -1, liveThreads = block->threadsSize;
    bool recentlyHalted = false;
    while (liveThreads) {
        if (!recentlyHalted && !block->threads[currentThread].finished && block->threads[currentThread].idleTimer == 0) { /// current thread isn't waiting
            SIM_MemInstRead(block->threads[currentThread].currInst, &inst, currentThread);

            switch (inst.opcode) {
                case CMD_NOP:
                    /// block->cycles++;
                    break;
                case CMD_ADD:
                    block->threads[currentThread].idleTimer++;
                    block->threads[currentThread].registers->reg[inst.dst_index] =
                            block->threads[currentThread].registers->reg[inst.src1_index] +
                            (inst.isSrc2Imm ? inst.src2_index_imm
                                            : block->threads[currentThread].registers->reg[inst.src2_index_imm]);
                    break;
                case CMD_SUB:
                    block->threads[currentThread].idleTimer++;
                    block->threads[currentThread].registers->reg[inst.dst_index] =
                            block->threads[currentThread].registers->reg[inst.src1_index] -
                            (inst.isSrc2Imm ? inst.src2_index_imm
                                            : block->threads[currentThread].registers->reg[inst.src2_index_imm]);
                    break;
                case CMD_ADDI:
                    block->threads[currentThread].idleTimer++;
                    block->threads[currentThread].registers->reg[inst.dst_index] =
                            block->threads[currentThread].registers->reg[inst.src1_index] +
                            (inst.isSrc2Imm ? inst.src2_index_imm
                                            : block->threads[currentThread].registers->reg[inst.src2_index_imm]);
                    break;
                case CMD_SUBI:
                    block->threads[currentThread].idleTimer++;
                    block->threads[currentThread].registers->reg[inst.dst_index] =
                            block->threads[currentThread].registers->reg[inst.src1_index] -
                            (inst.isSrc2Imm ? inst.src2_index_imm
                                            : block->threads[currentThread].registers->reg[inst.src2_index_imm]);
                    break;
                case CMD_LOAD:
                    block->threads[currentThread].idleTimer += block->loadLatency + 1;
                    SIM_MemDataRead(inst.src1_index + inst.src2_index_imm,
                                    &SIM_MemDataRead_ReturnValue); /// shouldn't this be the register value and not the index?
                    block->threads[currentThread].registers->reg[inst.dst_index] = SIM_MemDataRead_ReturnValue;
                    break;
                case CMD_STORE:
                    block->threads[currentThread].idleTimer += block->storeLatency + 1;
                    SIM_MemDataRead_ReturnValue = block->threads[currentThread].registers->reg[inst.src1_index];
                    SIM_MemDataWrite(inst.dst_index + inst.src2_index_imm,
                                     SIM_MemDataRead_ReturnValue);  /// shouldn't this be the register value and not the index?
                    break;
                case CMD_HALT:
                    block->threads[currentThread].idleTimer++;
                    block->threads[currentThread].finished = true;

                    bool allHalted = true; /// not needed
                    liveThreads--;
                    recentlyHalted = true;
                    break;
            }
            block->threads[currentThread].currInst++;
            block->instructions++;
            block->cycles++;

            for (int i = 0; i < block->threadsSize; i++) {
                block->threads[i].idleTimer = max(0, block->threads[i].idleTimer - 1);
            }


        } else { /// current thread is waiting
            bool foundReadyThread = false;

            int next_index = currentThread;
            while(true) {
                next_index++;
                if(next_index >= block->threadsSize) next_index = 0;

                if(next_index == currentThread) break;

                if(!block->threads[next_index].finished && !block->threads[next_index].idleTimer) {
                    if(!foundReadyThread) nextThread = next_index;
                    foundReadyThread = true;
                }
            }

            /// context switch
            if (foundReadyThread && (block->threads[currentThread].idleTimer || block->threads[currentThread].finished)) {
                previousThread = currentThread;
                currentThread = nextThread;

                /// reduce waiting time by switch overhead
                for (int i = 0; i < block->threadsSize; i++) {
                    block->threads[i].idleTimer = max(0, block->threads[i].idleTimer - block->switchOverhead);
                }
                block->cycles += block->switchOverhead;
            } else { /// all are waiting
                block->cycles++;
                for (int i = 0; i < block->threadsSize; i++) {
                    block->threads[i].idleTimer = max(0, block->threads[i].idleTimer - 1);
                }
            }
            recentlyHalted = false;
        }
    }
}

void CORE_FinegrainedMT() {

    fineGrained = new FineGrainedCore(SIM_GetThreadsNum());

    while (true) {
        bool stillAlive = false;
        bool allWaiting = true;
        for (int i = 0; i < fineGrained->threadsSize; i++) {
            if (fineGrained->threads[i].finished) {
                continue;
            } else {
                stillAlive = true;

                if (fineGrained->threads[i].idleTimer == 0) { // if idle timer is 0, perform next instruction

                    allWaiting = false;

                    SIM_MemInstRead(fineGrained->threads[i].currInst, &inst, i);
                    int SIM_MemDataRead_ReturnValue = 0;

                    switch (inst.opcode) {
                        case CMD_NOP:
                            /// fineGrained->cycles++;
                            break;
                        case CMD_ADD:
                            fineGrained->threads[i].idleTimer++;
                            fineGrained->threads[i].registers->reg[inst.dst_index] =
                                    fineGrained->threads[i].registers->reg[inst.src1_index] +
                                    (inst.isSrc2Imm ? inst.src2_index_imm
                                                    : fineGrained->threads[i].registers->reg[inst.src2_index_imm]);
                            break;
                        case CMD_SUB:
                            fineGrained->threads[i].idleTimer++;
                            fineGrained->threads[i].registers->reg[inst.dst_index] =
                                    fineGrained->threads[i].registers->reg[inst.src1_index] -
                                    (inst.isSrc2Imm ? inst.src2_index_imm
                                                    : fineGrained->threads[i].registers->reg[inst.src2_index_imm]);
                            break;
                        case CMD_ADDI:
                            fineGrained->threads[i].idleTimer++;
                            fineGrained->threads[i].registers->reg[inst.dst_index] =
                                    fineGrained->threads[i].registers->reg[inst.src1_index] +
                                    (inst.isSrc2Imm ? inst.src2_index_imm
                                                    : fineGrained->threads[i].registers->reg[inst.src2_index_imm]);
                            break;
                        case CMD_SUBI:
                            fineGrained->threads[i].idleTimer++;
                            fineGrained->threads[i].registers->reg[inst.dst_index] =
                                    fineGrained->threads[i].registers->reg[inst.src1_index] -
                                    (inst.isSrc2Imm ? inst.src2_index_imm
                                                    : fineGrained->threads[i].registers->reg[inst.src2_index_imm]);
                            break;
                        case CMD_LOAD:
                            fineGrained->threads[i].idleTimer += fineGrained->loadLatency + 1;
                            SIM_MemDataRead(inst.src1_index + inst.src2_index_imm,
                                            &SIM_MemDataRead_ReturnValue); /// shouldn't this be the register value and not the index?
                            fineGrained->threads[i].registers->reg[inst.dst_index] = SIM_MemDataRead_ReturnValue;
                            break;
                        case CMD_STORE:
                            fineGrained->threads[i].idleTimer += fineGrained->storeLatency + 1;
                            SIM_MemDataRead_ReturnValue = fineGrained->threads[i].registers->reg[inst.src1_index];
                            SIM_MemDataWrite(inst.dst_index + inst.src2_index_imm,
                                             SIM_MemDataRead_ReturnValue);  /// shouldn't this be the register value and not the index?
                            break;
                        case CMD_HALT:
                            fineGrained->threads[i].idleTimer++;
                            fineGrained->threads[i].finished = true;

                            bool allHalted = true;
                            for (int i = 0; i < fineGrained->threadsSize; i++) {
                                allHalted = allHalted && fineGrained->threads[i].finished;
                            }
                            if (allHalted) {
                                fineGrained->instructions++;
                                fineGrained->cycles++;
                                return;
                            }

                            break;
                    }
                    fineGrained->instructions++;
                    fineGrained->threads[i].currInst++;
                    fineGrained->cycles++;


                    for (int i = 0; i < fineGrained->threadsSize; i++) {
                        fineGrained->threads[i].idleTimer = max(fineGrained->threads[i].idleTimer - 1, 0);
                    }
                }
            }
        }

        if (allWaiting) {
            fineGrained->cycles++;
            for (int i = 0; i < fineGrained->threadsSize; i++) {
                fineGrained->threads[i].idleTimer = max(fineGrained->threads[i].idleTimer - 1, 0);
            }
        }

        // end if all threads are finished
        if (!stillAlive) break;
    }
}

double CORE_BlockedMT_CPI() {
    double BlockedMT_CPI = block->instructions ? (double) block->cycles / (double) block->instructions : 0;
    delete block;
    return BlockedMT_CPI;
}

double CORE_FinegrainedMT_CPI() {
    double FinegrainedMT_CPI = fineGrained->instructions ? (double) fineGrained->cycles / (double) fineGrained->instructions : 0;
    delete fineGrained;
    return FinegrainedMT_CPI;
}

void CORE_BlockedMT_CTX(tcontext *context, int threadid) {
    for (int i = 0; i < REGS_COUNT; i++) {
        context[threadid].reg[i] = block->threads[threadid].registers->reg[i];
    }
}

void CORE_FinegrainedMT_CTX(tcontext *context, int threadid) {
    for (int i = 0; i < REGS_COUNT; i++) {
        context[threadid].reg[i] = fineGrained->threads[threadid].registers->reg[i];
    }
}
