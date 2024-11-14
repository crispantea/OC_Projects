#include "L2_2Cache.h"

uint32_t createAddress(uint32_t tag, uint32_t index, uint32_t offset) {
  return ((tag << 14) | (index << 6) | offset);
}


void test0() {
  printf("-------- TEST 0 --------\n");

  uint8_t value, res;
  uint32_t address;
  int clock1, clock2, diff;
  long int total;

  resetTime();
  initCache();

  value = 4;
  total = 0;

  // Apenas coloca o valor 4 na RAM para futuros acessos
  address = 0x00008000;
  accessDRAM(address, &value, MODE_WRITE);
  address = 0x00000000;
  accessDRAM(address, &value, MODE_WRITE);


  value = 8;
  clock1 = getTime();
  write(address, &value);
  clock2 = getTime();
  diff = clock2 - clock1;
  total += diff;
  printf("Tempo: %d\n", diff);


  clock1 = getTime();
  read(address, &res);
  clock2 = getTime();
  diff = clock2 - clock1;
  total += diff;
  printf("Tempo: %d | Valor obtido: %d, Valor Correto: 8\n", diff, res);


  value = 6;
  address = 0x00008000;
  clock1 = getTime();
  write(address, &value);
  clock2 = getTime();
  diff = clock2 - clock1;
  total += diff;
  printf("Tempo: %d\n", diff);


  address = 0x00000000;
  clock1 = getTime();
  read(address, &res);
  clock2 = getTime();
  diff = clock2 - clock1;
  total += diff;
  printf("Tempo: %d | Valor obtido: %d, Valor Correto: 8\n", diff, res);


  address = 0x00008000;
  clock1 = getTime();
  read(address, &res);
  clock2 = getTime();
  diff = clock2 - clock1;
  total += diff;
  printf("Tempo: %d | Valor obtido: %d, Valor Correto: 6\n", diff, res);

  printf("Tempo Total em Operações: %ld\n", total);
}


void test1() {
    printf("-------- TEST 1 --------\n");
    
    int clock_previous = 0, value;
  
    resetTime();
    initCache();

    // Read on (0, 0, 0) -> Load from DRAM (100+10+1)
    read(createAddress(0, 0, 0), (unsigned char *)(&value));
    printf("Time: %d\n", (getTime() - clock_previous));
    clock_previous = getTime();

    // Read on (0, 0, 1) -> Load from L1 (1)
    read(createAddress(0, 0, 1), (unsigned char *)(&value));
    printf("Time: %d\n", (getTime() - clock_previous));
    clock_previous = getTime();

    // Read on (0, 1, 4) -> Load from DRAM (100+10+1)
    read(createAddress(0, 1, 4), (unsigned char *)(&value));
    printf("Time: %d\n", (getTime() - clock_previous));
    clock_previous = getTime();

    // Read on (1, 0, 0) -> Load from DRAM (100+10+1)
    read(createAddress(1, 0, 0), (unsigned char *)(&value));
    printf("Time: %d\n", (getTime() - clock_previous));
    clock_previous = getTime();

    // Read on (1, 0, 1) -> Load from L1 (1)
    read(createAddress(1, 0, 1), (unsigned char *)(&value));
    printf("Time: %d\n", (getTime() - clock_previous));
    clock_previous = getTime();

    // Read on (0, 0, 0) -> Load from L2 (10+1)
    read(createAddress(0, 0, 0), (unsigned char *)(&value));
    printf("Time: %d\n", (getTime() - clock_previous));
    clock_previous = getTime();

    // Read on (2, 0, 0) -> Load from DRAM (111)
    read(createAddress(2, 0, 0), (unsigned char *)(&value));
    printf("Time: %d\n", (getTime() - clock_previous));
    clock_previous = getTime();

    // Read on (1, 0, 0) -> Load from L2 (11)
    read(createAddress(1, 0, 0), (unsigned char *)(&value));
    printf("Time: %d\n", (getTime() - clock_previous));
    clock_previous = getTime();

    // Read on (3, 0, 0) -> Load from DRAM (111)
    read(createAddress(3, 0, 0), (unsigned char *)(&value));
    printf("Time: %d\n", (getTime() - clock_previous));
    clock_previous = getTime();

    // Read on (0, 0, 0) -> Load from DRAM (111)
    read(createAddress(0, 0, 0), (unsigned char *)(&value));
    printf("Time: %d\n", (getTime() - clock_previous));
    clock_previous = getTime();
}

void test2() {
    printf("-------- TEST 2 --------\n");
    
    int clock_previous = 0, value;
  
    resetTime();
    initCache();

    // Read on (0, 0, 0) -> Load from DRAM (111)
    read(createAddress(0, 0, 0), (unsigned char *)(&value));
    printf("Time: %d\n", (getTime() - clock_previous));
    clock_previous = getTime();

    // Read on (1, 0, 0) -> Load from DRAM (111)
    read(createAddress(1, 0, 0), (unsigned char *)(&value));
    printf("Time: %d\n", (getTime() - clock_previous));
    clock_previous = getTime();

    // Read on (2, 0, 0) -> Load from DRAM (111)
    read(createAddress(2, 0, 0), (unsigned char *)(&value));
    printf("Time: %d\n", (getTime() - clock_previous));
    clock_previous = getTime();

    // Read on (0, 0, 0) -> Load from L2 (11)
    read(createAddress(0, 0, 0), (unsigned char *)(&value));
    printf("Time: %d\n", (getTime() - clock_previous));
    clock_previous = getTime();
}


int main() {
  test0();
  
  return 0;
}