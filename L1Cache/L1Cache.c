#include "L1Cache.h"

uint8_t DRAM[DRAM_SIZE];
uint32_t time;
Cache L1Cache;

int L1_Offset_bits;
int L1_Index_bits;
int L1_Tag_bits;

/**************** Time Manipulation ***************/
void resetTime() { time = 0; }

uint32_t getTime() { return time; }

/****************  RAM memory (byte addressable) ***************/
void accessDRAM(uint32_t address, uint8_t *data, uint32_t mode) {

  if (address >= DRAM_SIZE - WORD_SIZE + 1)
    exit(-1);

  if (mode == MODE_READ) {
    memcpy(data, &(DRAM[address]), BLOCK_SIZE);
    time += DRAM_READ_TIME;
  }

  if (mode == MODE_WRITE) {
    memcpy(&(DRAM[address]), data, BLOCK_SIZE);
    time += DRAM_WRITE_TIME;
  }
}


/***************** L1 cache (byte addressable) ****************/
/**
 * Function used to initialize cache L1. All bits set to 0.
 */
void initCache() { 

  L1_Offset_bits = log2(WORD_PER_BLOCK) + 2;
  L1_Index_bits = log2(L1_BLOCKS);
  L1_Tag_bits = 32 - L1_Index_bits - L1_Offset_bits;

  L1Cache.init = 0;
  for (int i = 0; i < L1_BLOCKS; i++) {
    L1Cache.line[i].Dirty = 0;
    L1Cache.line[i].Valid = 0;
    L1Cache.line[i].Tag = 0;
    
    for (int j = 0; j < BLOCK_SIZE; j++) {
      L1Cache.line[i].slots[j] = 0;
    }
  }
}

/**
 * Function to get the offset value from an address.
 */
uint32_t getOffset(uint32_t address) {
  return address % (1 << L1_Offset_bits);
}

/**
 * Function to get the index value from an address.
 */
uint32_t getIndex(uint32_t address) {
    return ((address >> L1_Offset_bits) % (1 << L1_Index_bits));
}

/**
 * Function to get the tag value from an address.
 */
uint32_t getTag(uint32_t address) {
  return address >> (L1_Index_bits + L1_Offset_bits);
}

/**
 * Function to obtain the RAM address where the cache block (that is being
 * replaced) belongs to.
 * We use the tag (saved in cache) and the index (got from the new address).
 */
uint32_t getOldAddress(uint32_t address, uint32_t tag) {
  uint32_t index = getIndex(address);
  uint32_t offset = getOffset(address);

  uint32_t addressWithoutTag = (index << L1_Offset_bits) | offset;
  return (tag << (L1_Offset_bits + L1_Index_bits)) | addressWithoutTag;
}

/**
 * Function used to access L1 cache.
 */
void accessL1(uint32_t address, uint8_t *data, uint32_t mode) {
  uint32_t Tag = getTag(address);
  uint32_t Index = getIndex(address);
  uint32_t Offset = getOffset(address);

  uint8_t TempBlock[BLOCK_SIZE];

  CacheLine *Line = &L1Cache.line[Index];

  // Cache miss -> Replace with the correct block
  if (!Line->Valid || Line->Tag != Tag) {
    // Get the new block from L2 Cache
    accessDRAM(address - Offset, TempBlock, MODE_READ);

    // If line is dirty, store the information of that line
    // on the correct address in RAM
    if ((Line->Valid) && (Line->Dirty)) { // Line has dirty block
      // Get old address to write back
      uint32_t oldAddress = getOldAddress(address - Offset, Line->Tag);
      
      // Then write back old block
      accessDRAM(oldAddress, &Line->slots[0], MODE_WRITE);
    }

    // Copy new block to cache line
    memcpy(&Line->slots[0], TempBlock, BLOCK_SIZE);
    Line->Valid = 1;
    Line->Tag = Tag;
    Line->Dirty = 0;

  }

  if (mode == MODE_READ) {    // read data from cache line
    memcpy(data, &Line->slots[Offset], WORD_SIZE);
    time += L1_READ_TIME;
  }

  if (mode == MODE_WRITE) { // write data from cache line
    memcpy(&Line->slots[Offset], data, WORD_SIZE);
    time += L1_WRITE_TIME;
    Line->Dirty = 1;
  }
}

void read(uint32_t address, uint8_t *data) {
  accessL1(address, data, MODE_READ);
}

void write(uint32_t address, uint8_t *data) {
  accessL1(address, data, MODE_WRITE);
}
