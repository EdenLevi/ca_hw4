/* 046267 Computer Architecture - Winter 20/21 - HW #4 */

#ifndef _SIM_API_H_
#define _SIM_API_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>



/*********************************************/
/* The memory simulator API                  */
/*********************************************/
//freeing all the allocations
int SIM_MemFree();

/*! SIM_MemReset: Reset the memory simulator and load memory image
  \param[in] memImgFname Memory image filename
  The memory image filename is composed from segments of 2 types, defined by an "@" location/type line:
  1. "I@<address>" : The following lines are instructions at given memory offset.
     Each subsequent line up to the next "@" line is an instruction of format: <command> <dst>,<src1>,<src2>
     Commands is one of: NOP, ADD, SUB, LOAD, STORE
     operands are $<num> for any general purpose register, or just a number for immediate (for src2 only)
  2. "D@<address>" : The following lines are data values at given memory offset.
     Each subsequent line up the the next "@"is data value of a 32 bit (hex.) data word, e.g., 0x12A556FF
  \returns 0 - for success in reseting and loading image file. <0 in case of error.

  * Any memory address that is not defined in the given image file is initialized to zero.
 */
int SIM_MemReset(const char *memImgFname);

/*! SIM_ReadDataMem: Read data from main memory simulator
  \param[in] addr The memory location to read.
                  Note that while we read 32 bit data words, addressing is per byte, i.e., the address must be aligned to 4.
  \param[out] dst The destination location to read into
*/
void SIM_MemDataRead(uint32_t addr, int32_t *dst);

/*! SIM_MemDataWrite: Write a value to given memory address
  \param[in] addr The main memory address to write. Must be 4-byte-aligned
  \param[in] val  The value to write
*/
void SIM_MemDataWrite(uint32_t addr, int32_t val);

/*! SIM_ReadInstMem: Read instruction from main memory simulator
  \param[in] addr The memory location to read.
                  Note that while we read 32 bit data words, addressing is per byte, i.e., the address must be aligned to 4.
  \param[out] dst The destination location to read into
*/
void SIM_MemInstRead(uint32_t line, Instruction *dst, int tid);



/*********************************************/
/* Simulator parameters API                  */
/*********************************************/

/*! SIM_GetLoadLat: Get LOAD instruction latency as defined in the config file (L{x})
  \param[out] load latency cycles
*/
int SIM_GetLoadLat();

/*! SIM_GetStoreLat: Get STORE instruction latency as defined in the config file (S{x})
  \param[out] store latency cycles
*/
int SIM_GetStoreLat();

/*! SIM_GetSwitchCycles: Get the number of cycles it takes for context switch in blocked MT (O{x})
  \param[out] context switch latency cycles
*/
int SIM_GetSwitchCycles();

/*! SIM_GetThreadsNum: Get the number of threads
  \param[out] number of threads
*/
int SIM_GetThreadsNum();



#ifdef __cplusplus
}
#endif

#endif /*_SIM_API_H_*/
