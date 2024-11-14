#ifndef L1CACHE_H
#define L1CACHE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "../Cache.h"

#define L1_BLOCKS L1_SIZE/BLOCK_SIZE
#define WORD_PER_BLOCK (BLOCK_SIZE/WORD_SIZE)

void resetTime();

uint32_t getTime();

/****************  RAM memory (byte addressable) ***************/
void accessDRAM(uint32_t, uint8_t *, uint32_t);

/*********************** Cache *************************/

void initCache();
void initCacheL1();

uint32_t getOffset(uint32_t address);
uint32_t getIndex(uint32_t address);
uint32_t getTag(uint32_t address);
uint32_t getOldAddress(uint32_t address, uint32_t tag);

void accessL1(uint32_t address, uint8_t *data, uint32_t mode);

typedef struct CacheLine {
  uint8_t Valid;
  uint8_t Dirty;
  uint32_t Tag;
  uint8_t slots[BLOCK_SIZE];
} CacheLine;

typedef struct Cache {
  uint32_t init;
  CacheLine line[L1_BLOCKS];
} Cache;

/*********************** Interfaces *************************/

void read(uint32_t address, uint8_t *data);
void write(uint32_t address, uint8_t *data);

#endif
