#include <assert.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Virtual instruction set encoding.
// 1 byte per instruction: 2 bits for opcode, 3 bits each for 2 registers.
typedef uint8_t Instr;
#define NOPS (1 << 2)
#define NREGS (1 << 3)
#define REG_A_OFFSET 2
#define REG_B_OFFSET 5
#define GET_OP(instr) (instr & (NOPS - 1))
#define GET_REG_A(instr) (instr >> REG_A_OFFSET) & (NREGS - 1);
#define GET_REG_B(instr) (instr >> REG_B_OFFSET) & (NREGS - 1);
#define MOV 0
#define CMP 1
#define CMOVG 2
#define CMOVL 3
#define MKINSTR(op, a, b) (op | (a << REG_A_OFFSET) | (b << REG_B_OFFSET))

static void printinstr(Instr instr) {
  const uint8_t op = GET_OP(instr);
  const uint8_t a = GET_REG_A(instr);
  const uint8_t b = GET_REG_B(instr);
  const char *opnames[NOPS] = {"MOV", "CMP", "CMOVG", "CMOVL"};
  printf("%s r%d r%d\n", opnames[op], a, b);
}

static void printinstrs(const Instr *instrs, size_t instrs_len) {
  size_t i;
  for (i = 0; i < instrs_len; i++) {
    printinstr(instrs[i]);
  }
}

static int cmp_regs(const void *pa, const void *pb) {
  const uint64_t a = *(const uint64_t *)pa;
  const uint64_t b = *(const uint64_t *)pb;
  return a == b ? 0 : a < b ? -1 : 1;
}

static inline void runinstr(int *c, uint64_t *regs, Instr instr) {
  const uint8_t op = GET_OP(instr);
  const uint8_t a = GET_REG_A(instr);
  const uint8_t b = GET_REG_B(instr);
  switch (op) {
  case 0: // MOV a b
    regs[b] = regs[a];
    break;
  case 1: // CMP a b
    *c = cmp_regs(&regs[a], &regs[b]);
    break;
  case 2: // CMOVG a b
    if (*c > 0) {
      regs[b] = regs[a];
    }
    break;
  case 3: // CMOVL a b
    if (*c < 0) {
      regs[b] = regs[a];
    }
    break;
  default:
    assert(false);
    break;
  }
}

static inline void runinstrs(int *c, uint64_t *regs, const Instr *instrs,
                             size_t instrs_len) {
  size_t i;

#pragma clang loop unroll_count(12)
  for (i = 0; i < instrs_len; i++) {
    runinstr(c, regs, instrs[i]);
  }
}

// Run the instructions and check if they sort the input correctly.
static inline bool checksort(const Instr *instrs, size_t instrs_len,
                             const uint64_t *input, size_t input_len) {
  uint64_t regs[NREGS] = {0};
  uint64_t expected_output[NREGS];
  int c = 0;
  assert(input_len <= NREGS);
  memcpy(regs, input, input_len * sizeof(input[0]));
  memcpy(expected_output, input, input_len * sizeof(input[0]));
  qsort(expected_output, input_len, sizeof(input[0]), cmp_regs);
  runinstrs(&c, regs, instrs, instrs_len);
  return memcmp(expected_output, regs, input_len * sizeof(input[0])) == 0;
}

// Test if the instructions correctly sort an array of length 3.
// Tests all permutations.
static int checksort3(const Instr *instrs, size_t instrs_len) {
  static const uint64_t input0[3] = {1, 2, 3};
  static const uint64_t input1[3] = {1, 3, 2};
  static const uint64_t input2[3] = {2, 1, 3};
  static const uint64_t input3[3] = {2, 3, 1};
  static const uint64_t input4[3] = {3, 1, 2};
  static const uint64_t input5[3] = {3, 2, 1};

  static const uint64_t input6[3] = {1, 2, 2};
  static const uint64_t input7[3] = {2, 1, 2};
  static const uint64_t input8[3] = {2, 2, 1};

  static const uint64_t input9[3] = {1, 1, 1};

  return checksort(instrs, instrs_len, input0, 3) &&
         checksort(instrs, instrs_len, input1, 3) &&
         checksort(instrs, instrs_len, input2, 3) &&
         checksort(instrs, instrs_len, input3, 3) &&
         checksort(instrs, instrs_len, input4, 3) &&
         checksort(instrs, instrs_len, input5, 3) &&
         checksort(instrs, instrs_len, input6, 3) &&
         checksort(instrs, instrs_len, input7, 3) &&
         checksort(instrs, instrs_len, input8, 3) &&
         checksort(instrs, instrs_len, input9, 3);
}

extern int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  if (checksort3(data, size)) {
    printinstrs(data, size);
    // Trigger an error to tell libFuzzer that we found a solution.
    assert(false);
    return -1;
  }
  return 0;
}
