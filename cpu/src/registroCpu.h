#include <stdint.h>

typedef struct {
    uint8_t AX;
    uint8_t BX;
    uint8_t CX;
    uint8_t DX;
    uint32_t EAX;
    uint32_t EBX;
    uint32_t ECX;
    uint32_t EDX;
    uint32_t SI;
    uint32_t DI;
    uint32_t PC;
} registros_cpu;

typedef enum {
  AX,
  BX,
  CX,
  DX,
  EAX,
  EBX,
  ECX,
  EDX,
  SI,
  DI,
  PC
} registrosCPU;

void init_cpu_registers();

extern registros_cpu miCPU; 