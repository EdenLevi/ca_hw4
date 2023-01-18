/* 046267 Computer Architecture - Winter 20/21 - HW #4 */
/* Main memory simulator implementation                */

#include "core_api.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

static const char *cmdStr[] = {"NOP", "ADD", "SUB","ADDI", "SUBI","LOAD", "STORE", "HALT"};
uint32_t prog_start; // the addr of the code block
uint32_t data_start; // the addr of the data block
Instruction** instructions; // where the instructions are kept
int32_t data[100]; // where the data is kept
uint32_t ticks; // the current clk tick
uint32_t read_tick; // the clk tick of the first attempt to read
uint32_t write_tick;// the clk tick for write
int load_store_latency[2];//load store
int switch_; //the cycles that switch between cycles takes
int threadnumber;

typedef struct {
    uint32_t addr;
    int32_t val;
    bool valid;
    uint32_t ticks; // for LRU
} cache_line;


uint32_t get_start(char *line) {
    line = strtok(line, "\n");
    strtok(line, "@");
    line = strtok(NULL, "@");
    return (uint32_t) strtol(line, NULL, 0);
}

void get_data(char *line, int data_i) {
    line = strtok(line, "\n");
    data[data_i] = (int32_t) strtol(line, NULL, 0);
}

int get_dst(char *dst) {
    strtok(dst, ",");
    strtok(dst, "$");
    dst = strtok(NULL, "$");
    return atoi(dst);
}

int get_dst_br(char *dst) {
    strtok(dst, "\n");
    strtok(dst, "$");
    dst = strtok(NULL, "$");
    return atoi(dst);
}

int get_src1(char *src1) {
    strtok(src1, ",");
    src1 = strtok(NULL, ",");
    strtok(src1, "$");
    src1 = strtok(NULL, "$");
    return atoi(src1);
}

int get_src2(char *src2) {
    strtok(src2, ",");
    strtok(NULL, ",");
    src2 = strtok(NULL, ",");
    strtok(src2, "$");
    src2 = strtok(NULL, "$");
    src2 = strtok(src2, "\n");
    return atoi(src2);
}

int get_src2_imm(char *src2, int inst_num, int tid) {
	instructions[tid][inst_num].isSrc2Imm = 0; //assert
    strtok(src2, ",");
    strtok(NULL, ",");
    src2 = strtok(NULL, ",");
    if (strchr(src2, '$') == NULL) {
        strtok(src2, " ");
        instructions[tid][inst_num].isSrc2Imm = 1;
    } else {
        strtok(src2, "$");
        src2 = strtok(NULL, "$");
        assert(instructions[tid][inst_num].isSrc2Imm == 0);
    }
    src2 = strtok(src2, "\n");
    if (strchr(src2, 'x') == NULL) {
        return atoi(src2);
    } else {
        return (uint32_t) strtol(src2, NULL, 0);
    }
}

void add_sub(char *line, int inst_num, int tid) {
    char dst[50];
    instructions[tid][inst_num].isSrc2Imm = 0;
    memset(dst, '\0', sizeof(dst));
    strcpy(dst, line);
    instructions[tid][inst_num].dst_index = get_dst(dst);
    char src1[50];
    memset(src1, '\0', sizeof(src1));
    strcpy(src1, line);
    instructions[tid][inst_num].src1_index = get_src1(src1);
    char src2[50];
    memset(src2, '\0', sizeof(src2));
    strcpy(src2, line);
    instructions[tid][inst_num].src2_index_imm = get_src2_imm(src2, inst_num, tid);
}

void halt(char *line, int inst_num, int tid) {
    char dst[50];
    memset(dst, '\0', sizeof(dst));
    strcpy(dst, line);
    instructions[tid][inst_num].dst_index = get_dst(dst);
    instructions[tid][inst_num].isSrc2Imm=0;
    instructions[tid][inst_num].src1_index=0;
    instructions[tid][inst_num].src2_index_imm=0;
}


void load_store(char *line, int inst_num, int tid) {
    char dst[50];
    memset(dst, '\0', sizeof(dst));
    strcpy(dst, line);
    instructions[tid][inst_num].dst_index = get_dst(dst);
    char src1[50];
    memset(src1, '\0', sizeof(src1));
    strcpy(src1, line);
    instructions[tid][inst_num].src1_index = get_src1(src1);
    char src2[50];
    memset(src2, '\0', sizeof(src2));
    strcpy(src2, line);
    instructions[tid][inst_num].src2_index_imm = get_src2_imm(src2, inst_num, tid);
}


void get_inst(char *line, int inst_num, int tid) {
    char command[50];
    memset(command, '\0', sizeof(command));
    strcpy(command, line);
    strtok(command, " ");
    int opc = 0;
    while (strcmp(command, cmdStr[opc]) != 0) {
        ++opc;
    }
    instructions[tid][inst_num].opcode = opc;
    switch (opc) {
        case CMD_NOP: // NOP
            break;
        case CMD_ADDI:
        case CMD_SUBI:
            add_sub(line, inst_num, tid);
            break;
        case CMD_ADD:
        case CMD_SUB:
            add_sub(line, inst_num, tid);
            break;
        case CMD_LOAD:
        case CMD_STORE:
            load_store(line, inst_num, tid);
            break;
        case CMD_HALT:
            halt(line, inst_num, tid);
            break;
    }
}

int SIM_MemReset(const char *memImgFname) {
    FILE *img = fopen(memImgFname, "r");
    int tid;
    char line[1024];
    if (img == 0) {
        return -1; // can't open img file
    }
    while (fgets(line, 1024, img) != NULL) {
        if (line[0] == '#' || line[0] == '\n')   // comment or empty line
        {
            continue;
        }
        if(line[0] == 'S') {
        	load_store_latency[1]=atoi(&line[1]);
        	continue;
        }
        if(line[0] == 'L') {
        	load_store_latency[0]=atoi(&line[1]);
        	continue;
        }
        if(line[0] == 'O') {
        	switch_=atoi(&line[1]);
        	continue;
        }
        if(line[0] == 'N'){
			threadnumber=atoi(&line[1]);
			instructions = malloc(sizeof(*instructions)*threadnumber);
			for(int i=0; i<threadnumber; i++){
				instructions[i]=malloc(sizeof(instructions[i])*100);
			}
			break;
		}
    }

    while (fgets(line, 1024, img) != NULL) {
        if (line[0] == '#' || line[0] == '\n')   // comment or empty line
        {
            continue;
        }
        if(line[0] == 'T'){
        	tid=atoi(&line[1]);
        }
        else if (line[0] == 'I' && line[1] == '@')     // start of code block
        {
            prog_start = get_start(line);
            int inst = 0;
            fgets(line, 1024, img);
            // get next instructions
            while (line[0] != '\n' && line[0] != '#' && line[0] != 'D') {
                get_inst(line, inst, tid);
                ++inst;
                if (fgets(line, 1024, img) == NULL)   //EOF
                {
                    break;
                }
            }
        } else if (line[0] == 'D' && line[1] == '@')     // start of data block
        {
            data_start = get_start(line);
            int data_i = 0;
            fgets(line, 1024, img);
            while (line[0] != '\n' && line[0] != '#' && line[0] != 'I') {
                get_data(line, data_i);
                ++data_i;
                if (fgets(line, 1024, img) == NULL) {
                    break;
                }
            }
        }
    }
    fclose(img);
    return 0;
}

void SIM_MemFree(){
	for(int i=0; i<threadnumber; i++){
		free(instructions[i]);
	}
	free(instructions);
}

void SIM_MemDataRead(uint32_t addr, int32_t *dst) {
    int addr_i = addr - data_start;
    addr_i = addr_i / 4;
    *dst = data[addr_i];
}

void SIM_MemDataWrite(uint32_t addr, int32_t val) {
    int addr_i = addr - data_start;
    addr_i = addr_i / 4; // addr is aligned to 4 byte
    data[addr_i] = val;
}

void SIM_MemInstRead(uint32_t line, Instruction *dst, int tid) {
    dst->opcode = instructions[tid][line].opcode;
    dst->dst_index = instructions[tid][line].dst_index;
    dst->src1_index = instructions[tid][line].src1_index;
    dst->src2_index_imm = instructions[tid][line].src2_index_imm;
    dst->isSrc2Imm = instructions[tid][line].isSrc2Imm;
}

int SIM_GetLoadLat() {
    return load_store_latency[0];
}

int SIM_GetStoreLat() {
    return load_store_latency[1];
}

int SIM_GetThreadsNum() {
	return threadnumber;
}

int SIM_GetSwitchCycles() {
    return switch_;
}
