#include "L2_2Cache.h"

uint8_t DRAM[DRAM_SIZE];
uint32_t time;
CacheL1 L1Cache;
CacheL2 L2Cache;

int L1_Offset_bits;
int L1_Index_bits;
int L1_Tag_bits;

int L2_Offset_bits;
int L2_Index_bits;
int L2_Tag_bits;

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


/************ L1 and L2 Caches (byte addressable) **************/
/**
 * Function used to initialize both L1 and L2 caches at once.
 */
void initCache() {
  L1_Offset_bits = log2(WORD_PER_BLOCK) + 2;
  L1_Index_bits = log2(L1_BLOCKS);
  L1_Tag_bits = 32 - L1_Index_bits - L1_Offset_bits;

  L2_Offset_bits = log2(WORD_PER_BLOCK) + 2;
  L2_Index_bits = log2(L2_BLOCKS/WAYS);
  L2_Tag_bits = 32 - L2_Index_bits - L2_Offset_bits;

  initCacheL1();
  initCacheL2();
}

/**
 * Function used to initialize cache L1. All bits set to 0.
 */
void initCacheL1() { 
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
 * Function used to initialize cache L2. All bits set to 0, except for each
 * line1 is_next parameter (set to 1 -> TRUE).
 */
void initCacheL2() { 
  for (int i = 0; i < L2_BLOCKS/WAYS; i++) {
    for (int k = 0; k < WAYS; k++) {
      L2Cache.sets[k].line[i].Dirty = 0;
      L2Cache.sets[k].line[i].Valid = 0;
      L2Cache.sets[k].line[i].Tag = 0;
      L2Cache.sets[k].line[i].time = 0;

      for (int j = 0; j < BLOCK_SIZE; j++) {
        L2Cache.sets[k].line[i].slots[j] = 0;
        L2Cache.sets[k].line[i].slots[j] = 0;
      }
    }
  }
}

/**
 * Function to get the offset value from an address.
 */
uint32_t getOffset(uint32_t address, int CacheType) {
  int offsetBits;
  switch (CacheType)
  {
  case L1CACHE:
    offsetBits = L1_Offset_bits;
    break;
  
  case L2CACHE:
    offsetBits = L2_Offset_bits;
    break;
  };
  
  return address % (1 << offsetBits);
}

/**
 * Function to get the index value from an address.
 */
uint32_t getIndex(uint32_t address, int CacheType) {
  int offsetBits;
  int indexBits;
  switch (CacheType)
  {
  case L1CACHE:
    offsetBits = L1_Offset_bits;
    indexBits = L1_Index_bits;
    break;

  case L2CACHE:
    offsetBits = L2_Offset_bits;
    indexBits = L2_Index_bits;
    break;
  };

  return ((address >> offsetBits) % (1 << indexBits));
}

/**
 * Function to get the tag value from an address.
 */
uint32_t getTag(uint32_t address, int CacheType) {
  int offsetBits;
  int indexBits;
  switch (CacheType)
  {
  case L1CACHE:
    offsetBits = L1_Offset_bits;
    indexBits = L1_Index_bits;
    break;
  
  case L2CACHE:
    offsetBits = L2_Offset_bits;
    indexBits = L2_Index_bits;
    break;
  };
  return address >> (indexBits + offsetBits);
}

/**
 * Function to obtain the RAM address where the cache block (that is being
 * replaced) belongs to.
 * We use the tag (saved in cache) and the index (got from the new address).
 * The offset is set to 0 (6 less relevant bits).
 */
uint32_t getOldAddress(uint32_t address, uint32_t tag, int CacheType) {
  uint32_t index = getIndex(address, CacheType);
  uint32_t offset = getOffset(address, CacheType);

  int offsetBits;
  int indexBits;
  switch (CacheType)
  {
  case L1CACHE:
    offsetBits = L1_Offset_bits;
    indexBits = L1_Index_bits;
    break;
  
  case L2CACHE:
    offsetBits = L2_Offset_bits;
    indexBits = L2_Index_bits;
    break;
  };

  uint32_t addressWithoutTag = (index << offsetBits) | offset;
  return (tag << (offsetBits + indexBits)) | addressWithoutTag;
}


/**
 * Function used to access L1 cache.
 */
void accessL1(uint32_t address, uint8_t *data, uint32_t mode) {
  uint32_t Tag = getTag(address, L1CACHE);
  uint32_t Index = getIndex(address, L1CACHE);
  uint32_t Offset = getOffset(address, L1CACHE);

  uint8_t TempBlock[BLOCK_SIZE];
  
  CacheLine *Line = &L1Cache.line[Index];

  // Cache miss -> Replace with the correct block
  if (!Line->Valid || Line->Tag != Tag) {
    // Get the new block from L2 Cache
    accessL2(address - Offset, TempBlock, MODE_READ);

    // If line is dirty, store the information of that line
    // on the correct address in CacheL2
    if ((Line->Valid) && (Line->Dirty)) {
      // Get the correct address. Why?
      //  - Because the current address has a tag that doesn't match the tag 
      //    currently in the cache, and the information that's not updated 
      //    corresponds to the address with the tag currently stored in the cache.
      uint32_t oldAddress = getOldAddress(address - Offset, Line->Tag, L1CACHE);
      // Then write back old block
      accessL2(oldAddress, &Line->slots[0], MODE_WRITE);
    }

    // Stores the information retrieved from Cache L2
    memcpy(&Line->slots[0], TempBlock, BLOCK_SIZE);
    Line->Valid = 1;
    Line->Tag = Tag;
    Line->Dirty = 0;
  }

  if (mode == MODE_READ) {
    memcpy(data, &Line->slots[Offset], WORD_SIZE);
    time += L1_READ_TIME;
  }

  if (mode == MODE_WRITE) {
    memcpy(&Line->slots[Offset], data, WORD_SIZE);
    time += L1_WRITE_TIME;
    Line->Dirty = 1;
  }
}

/**
 * Function used to access L2 cache.
 */
void accessL2(uint32_t address, uint8_t *data, uint32_t mode) {
  uint32_t Index = getIndex(address, L2CACHE);
  uint32_t Tag = getTag(address, L2CACHE);
  uint8_t TempBlock[BLOCK_SIZE];

  CacheLine *Line = &L2Cache.sets[Index].line[0];
  int oldestTime = INT8_MAX;

  // Search for the cache line to use (either a hit or the oldest one for replacement)
  for (int i = 0; i < WAYS; i++) {
    CacheLine *CurrentLine = &L2Cache.sets[Index].line[i];

    // If the tag matches and the line is valid, it's a hit, so we use this line
    if (CurrentLine->Valid && CurrentLine->Tag == Tag) {
      Line = CurrentLine;
      break;  // Exit the loop, as we found the correct line (hit)
    }

    // If not a hit, track the oldest line based on the time field.
    // If no hit is found by the end of the loop, we will use the oldest line for replacement.
    if (CurrentLine->time < oldestTime) {
      oldestTime = CurrentLine->time;
      Line = CurrentLine;
    }
  }

  // Cache miss -> Replace with the correct block
  if (!Line->Valid || Line->Tag != Tag) {
    // Get the new block from DRAM
    accessDRAM(address, TempBlock, MODE_READ);


    // If line is dirty, store the information of that line
    // on the correct address in CacheL2
    if (Line->Dirty) {
      // Get the correct address. Why?
      //  - Because the current address has a tag that doesn't match the tag 
      //    currently in the cache, and the information that's not updated 
      //    corresponds to the address with the tag currently stored in the cache.
      uint32_t oldAddress = getOldAddress(address, Line->Tag, L2CACHE);
      // Then write back old block
      accessDRAM(oldAddress, Line->slots, MODE_WRITE);
    }

    // Stores the information retrieved from Cache L2
    Line->Valid = 1;
    Line->Tag = Tag;
    Line->Dirty = 0;
    memcpy(&Line->slots[0], TempBlock, BLOCK_SIZE);
  }

  /* Faz a leitura ou escrita de acordo com o modo */
  if (mode == MODE_READ) {
    memcpy(data, &Line->slots[0], BLOCK_SIZE);
    Line->Dirty = 0;
    time += L2_READ_TIME;
  } 

  if (mode == MODE_WRITE) {
    memcpy(&Line->slots[0], data, BLOCK_SIZE);
    Line->Dirty = 1;
    time += L2_WRITE_TIME;
  }
   Line->time = getTime();
}

void read(uint32_t address, uint8_t *data) {
  accessL1(address, data, MODE_READ);
}

void write(uint32_t address, uint8_t *data) {
  accessL1(address, data, MODE_WRITE);
}
