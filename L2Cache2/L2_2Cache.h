#ifndef L2_2CACHE_H
#define L2_2CACHE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "../Cache.h"

#define L1_BLOCKS L1_SIZE/BLOCK_SIZE
#define L2_BLOCKS L2_SIZE/BLOCK_SIZE
#define WORD_PER_BLOCK (BLOCK_SIZE/WORD_SIZE)

#define L1CACHE 1
#define L2CACHE 2

#define WAYS 2

#define FALSE 0
#define TRUE  1

void resetTime();

uint32_t getTime();

/****************  RAM memory (byte addressable) ***************/
void accessDRAM(uint32_t address, uint8_t *data, uint32_t mode);

/*********************** Cache *************************/

void initCache();
void initCacheL1();
void initCacheL2();

uint32_t getOffset(uint32_t address, int CacheType);
uint32_t getIndex(uint32_t address, int CacheType);
uint32_t getTag(uint32_t address, int CacheType);
uint32_t getOldAddress(uint32_t address, uint32_t tag, int CacheType);

void accessL1(uint32_t address, uint8_t *data, uint32_t mode);
void accessL2(uint32_t address, uint8_t *data, uint32_t mode);


typedef struct CacheLine {
  uint8_t Valid;
  uint8_t Dirty;
  uint32_t Tag;
  uint8_t slots[BLOCK_SIZE];
  int time;
  uint8_t is_next;
} CacheLine;

typedef struct CacheL1 {
  uint32_t init;
  CacheLine line[L1_BLOCKS];
} CacheL1;

typedef struct L2Set {
  CacheLine line[WAYS];
} L2Set;

typedef struct CacheL2 {
  uint32_t init;
  L2Set sets[L2_BLOCKS/WAYS];
} CacheL2;

/*********************** Interfaces *************************/

void read(uint32_t address, uint8_t *data);
void write(uint32_t address, uint8_t *data);

#endif
