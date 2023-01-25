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

};

class MT_core {
public:
    int loadLatency;
    int storeLatency;
    int threadsSize;
    thread *threads;
    int cycles;
    int instructions; // CPI will be calculated based on this
    int RR; // initialized to thread 0

    MT_core(int threadsSize, thread *threads, int cycles, int instructions) : threadsSize(threadsSize),
                                                                              threads(threads), cycles(cycles),
                                                                              instructions(instructions) {
        loadLatency = SIM_GetLoadLat();
        storeLatency = SIM_GetStoreLat();
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

    BlockCore(int threadsSize, thread *threads, int cycles, int instructions) : MT_core(threadsSize, threads, cycles,
                                                                                        instructions) {
        switchOverhead = SIM_GetSwitchCycles(); // relevant only for BLOCK
        cout << "block constructed" << endl;
    }

    ~BlockCore() {
        cout << "block destructed" << endl;
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
    FineGrainedCore(int threadsSize, thread *threads, int cycles, int instructions) : MT_core(threadsSize, threads,
                                                                                              cycles, instructions) {
        cout << "fine grained constructed" << endl;
    }

    ~FineGrainedCore() {
        cout << "fine grained destructed" << endl;
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

BlockCore block = BlockCore(SIM_GetThreadsNum(), nullptr, SIM_GetSwitchCycles(), 0); // (int threadsSize, thread *threads, int cycles, int instructions)
FineGrainedCore fineGrained = FineGrainedCore();

// run a simulation of BlockCore
void CORE_BlockedMT() {
    while (true) {
        bool stillAlive = false;
        for (int i = 0; i < block.threadsSize; i++) {
            if (block.threads[(block.RR + i) % block.threadsSize].finished) {
                continue;
            } else stillAlive = true;

            /// do stuff here

            /// *************
        }
        // end if all threads are finished
        if (!stillAlive) break;
    }
}

void CORE_FinegrainedMT() {
    while (true) {
        bool stillAlive = false;
        for (int i = 0; i < fineGrained.threadsSize; i++) {
            if (fineGrained.threads[(fineGrained.RR + i) % fineGrained.threadsSize].finished) {
                continue;
            } else {
                stillAlive = true;
                fineGrained.instructions++;
            }

            /// do stuff here

            /// *************
        }
        // end if all threads are finished
        if (!stillAlive) break;
    }
}

double CORE_BlockedMT_CPI() {
    double BlockedMT_CPI = block.cycles / block.instructions;

    // can release memory here
    return BlockedMT_CPI;
}

double CORE_FinegrainedMT_CPI() {
    double FinegrainedMT = block.cycles / block.instructions;

    return FinegrainedMT;
}

void CORE_BlockedMT_CTX(tcontext *context, int threadid) {

}

void CORE_FinegrainedMT_CTX(tcontext *context, int threadid) {
    for(int i = 0; i < REGS_COUNT; i++) {
        context[i] = fineGrained.threads[threadid].registers[i];
    }
}
