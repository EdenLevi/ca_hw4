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

        cout << "thread constructed" << endl;

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
        cout << "MT_core constructed" << endl;
    }

    virtual ~MT_core() {
        delete[] threads;
        cout << "MT_core destructed" << endl;
    }

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
        cout << "block constructed" << endl;
    }

    ~BlockCore() {
        cout << "block destructed" << endl;
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
    FineGrainedCore(int threadsSize) : MT_core(threadsSize) {
        cout << "fine grained constructed" << endl;
    }

    ~FineGrainedCore() {
        cout << "fine grained destructed" << endl;
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

    cout << block->threadsSize << endl;
    int SIM_MemDataRead_ReturnValue, currentThread = 0, previousThread = 0, nextThread = -1, liveThreads = block->threadsSize;
    while (liveThreads) {
        cout << "WE ARE HERE\n";
        if (block->threads[currentThread].finished) {
            currentThread = (currentThread + 1) % block->threadsSize;
            continue;
        } else { /// current thread is alive
            if (block->threads[currentThread].idleTimer != 0) { /// current thread isn't waiting
                if (block->threads[currentThread].idleTimer != 0) {
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
                            block->threads[currentThread].idleTimer += fineGrained->loadLatency + 1;
                            SIM_MemDataRead(inst.src1_index + inst.src2_index_imm,
                                            &SIM_MemDataRead_ReturnValue); /// shouldn't this be the register value and not the index?
                            block->threads[currentThread].registers->reg[inst.dst_index] = SIM_MemDataRead_ReturnValue;
                            break;
                        case CMD_STORE:
                            block->threads[currentThread].idleTimer += fineGrained->storeLatency + 1;
                            SIM_MemDataRead_ReturnValue = block->threads[currentThread].registers->reg[inst.src1_index];
                            SIM_MemDataWrite(inst.dst_index + inst.src2_index_imm,
                                             SIM_MemDataRead_ReturnValue);  /// shouldn't this be the register value and not the index?
                            break;
                        case CMD_HALT:
                            block->threads[currentThread].idleTimer++;
                            block->threads[currentThread].finished = true;

                            bool allHalted = true; /// not needed
                            liveThreads--;
                            break;
                    }

                }


            } else { /// current thread is waiting
                bool foundReadyThread = false;
                for (int j = 0; (!foundReadyThread) && j < block->threadsSize; j++) {
                    if (block->threads[(currentThread + j) % (block->threadsSize)].idleTimer ==
                        0) { /// found a ready thread
                        if (foundReadyThread == false) nextThread = currentThread +
                                                                    (currentThread + j) % (block->threadsSize);
                        foundReadyThread = true;
                    }
                }


            }
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
                                allHalted = fineGrained->threads[i].finished;
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
                    cout << "OPCODE: " << inst.opcode << endl;


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
            cout << "OPCODE: " << inst.opcode << " (all done)" << endl;
        }

        // end if all threads are finished
        if (!stillAlive) break;
    }
}

double CORE_BlockedMT_CPI() {
    double BlockedMT_CPI = block->instructions ? block->cycles / block->instructions : 0;

    //delete block;
    return BlockedMT_CPI;
}

double CORE_FinegrainedMT_CPI() {
    cout << "FinegrainedMT_CPI = " << fineGrained->cycles << " / " << fineGrained->instructions << endl;
    double FinegrainedMT_CPI = fineGrained->instructions ? (double) fineGrained->cycles /
                                                           (double) fineGrained->instructions : 0;

    //delete fineGrained;
    return FinegrainedMT_CPI;
}

void CORE_BlockedMT_CTX(tcontext *context, int threadid) {
    for (int i = 0; i < REGS_COUNT; i++) {
        context->reg[i] = block->threads[threadid].registers->reg[i];
    }
}

void CORE_FinegrainedMT_CTX(tcontext *context, int threadid) {
    for (int i = 0; i < REGS_COUNT; i++) {
        //cout << "reg[" << i << "]: " << fineGrained->threads[threadid].registers->reg[i] << endl;
        //context->reg[i] = fineGrained->threads[threadid].registers->reg[i];
        context[threadid].reg[i] = fineGrained->threads[threadid].registers->reg[i];
    }
}
