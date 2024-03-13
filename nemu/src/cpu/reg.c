#include "nemu.h"
#include <stdlib.h>
#include <time.h>

CPU_state cpu;

const char *regsl[] = {"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"};
const char *regsw[] = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};
const char *regsb[] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};

void reg_test() {
  srand(time(0));
  uint32_t sample[8];
  uint32_t eip_sample = rand();
  cpu.eip = eip_sample;

  int i;
  for (i = R_EAX; i <= R_EDI; i ++) {
    sample[i] = rand();
    reg_l(i) = sample[i];
    assert(reg_w(i) == (sample[i] & 0xffff));
  }

  assert(reg_b(R_AL) == (sample[R_EAX] & 0xff));
  assert(reg_b(R_AH) == ((sample[R_EAX] >> 8) & 0xff));
  assert(reg_b(R_BL) == (sample[R_EBX] & 0xff));
  assert(reg_b(R_BH) == ((sample[R_EBX] >> 8) & 0xff));
  assert(reg_b(R_CL) == (sample[R_ECX] & 0xff));
  assert(reg_b(R_CH) == ((sample[R_ECX] >> 8) & 0xff));
  assert(reg_b(R_DL) == (sample[R_EDX] & 0xff));
  assert(reg_b(R_DH) == ((sample[R_EDX] >> 8) & 0xff));

  assert(sample[R_EAX] == cpu.eax);
  assert(sample[R_ECX] == cpu.ecx);
  assert(sample[R_EDX] == cpu.edx);
  assert(sample[R_EBX] == cpu.ebx);
  assert(sample[R_ESP] == cpu.esp);
  assert(sample[R_EBP] == cpu.ebp);
  assert(sample[R_ESI] == cpu.esi);
  assert(sample[R_EDI] == cpu.edi);

  assert(eip_sample == cpu.eip);
}

uint32_t reg_value(char *reg) {
    //增加一个和eip的比较,因为寄存器名还可能是eip,此时返回cpu.eip
    if (strcmp(reg, "eip") == 0) {
        return cpu.eip;
    }

    // Check for 32-bit registers
    for (int i = 0; i < 8; ++i) {
        if (strcmp(reg, regsl[i]) == 0) {
            return reg_l(i);
        }
    }

    // Check for 16-bit registers
    for (int i = 0; i < 8; ++i) {
        if (strcmp(reg, regsw[i]) == 0) {
            return reg_w(i);
        }
    }

    // Check for 8-bit registers
    for (int i = 0; i < 8; ++i) {
        if (strcmp(reg, regsb[i]) == 0) {
            return reg_b(i);
        }
    }

    // If the register name does not match, handle the error appropriately
    // For now, we just return 0 and possibly log an error message
    fprintf(stderr, "Invalid register name: %s\n", reg);
    return 0;
}