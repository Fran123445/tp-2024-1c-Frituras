#ifndef REGISTROCPU_H_
#define REGISTROCPU_H_

#include <stdio.h>
#include <stdint.h> 
#include <stddef.h>

// Declaraciones de tipos de datos
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

typedef struct {
    // Definición de los registros de la CPU
    uint8_t AX, BX, CX, DX;
    uint32_t EAX, EBX, ECX, EDX;
    uint32_t SI, DI, PC;
} registros_cpu;


// Declaraciones de variables globales externas
extern registros_cpu miCPU;

#endif /* REGISTROCPU_H_ */