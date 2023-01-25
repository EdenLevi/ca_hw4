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

    thread() : finished(false), currInst(0) {

        cout << "thread constructed" << endl;

        registers = new tcontext();

        for(int i = 0; i < REGS_COUNT; i++) {
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

BlockCore* block;
FineGrainedCore* fineGrained;
Instruction inst;

// run a simulation of BlockCore
void CORE_BlockedMT() {

    block = new BlockCore(SIM_GetThreadsNum());

    while (true) {
        bool stillAlive = false;
        for (int i = 0; i < block->threadsSize; i++) {
            if (block->threads[i].finished) {
                continue;
            } else {
                stillAlive = true;
                block->instructions++;
            }

            /// do stuff here

            /// *************
        }
        // end if all threads are finished
        if (!stillAlive) break;
    }
}

void CORE_FinegrainedMT() {

    fineGrained = new FineGrainedCore(SIM_GetThreadsNum());

    while (true) {
        bool stillAlive = false;
        for (int i = 0; i < fineGrained->threadsSize; i++) {
            if (fineGrained->threads[i].finished) {
                continue;
            } else {
                stillAlive = true;

                SIM_MemInstRead(fineGrained->threads[i].currInst, &inst, i);
                switch(inst.opcode) {
                    case CMD_NOP:
                        fineGrained->cycles++;
                        break;
                    case CMD_ADD:
                        fineGrained->cycles++;
                        fineGrained->threads[i].registers->reg[inst.dst_index] = fineGrained->threads[i].registers->reg[inst.src1_index] + fineGrained->threads[i].registers->reg[inst.src2_index_imm];
                        break;
                    case CMD_SUB:
                        break;
                    case CMD_ADDI:
                        break;
                    case CMD_SUBI:
                        break;
                    case CMD_LOAD:
                        fineGrained->cycles += fineGrained->loadLatency + 1;
                        break;
                    case CMD_STORE:
                        fineGrained->cycles += fineGrained->storeLatency + 1;
                        break;
                    case CMD_HALT:
                        fineGrained->cycles++;
                        break;
                }
                fineGrained->instructions++;
            }
        }
        // end if all threads are finished
        if (!stillAlive) break;
    }
}

double CORE_BlockedMT_CPI() {
    double BlockedMT_CPI = block->cycles / block->instructions;

    delete block;
    return BlockedMT_CPI;
}

double CORE_FinegrainedMT_CPI() {
    double FinegrainedMT_CPI = block->cycles / block->instructions;

    delete fineGrained;
    return FinegrainedMT_CPI;
}

void CORE_BlockedMT_CTX(tcontext *context, int threadid) {
    for(int i = 0; i < REGS_COUNT; i++) {
        context[i] = block->threads[threadid].registers[i];
    }
}

void CORE_FinegrainedMT_CTX(tcontext *context, int threadid) {
    for(int i = 0; i < REGS_COUNT; i++) {
        context[i] = fineGrained->threads[threadid].registers[i];
    }
}
