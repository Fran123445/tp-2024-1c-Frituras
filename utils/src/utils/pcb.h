#ifndef PCB_H
#define PCB_H

#include <stdlib.h>
#include <stdint.h> // Incluye esta línea si utilizas tipos de datos como uint8_t o uint32_t
#include <stdio.h>

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

typedef enum {
    NEW,
    READY,
    BLOCKED,
    EXEC,
    EXIT
}estado_proceso;

typedef struct {
    int PID;
    uint32_t programCounter;
    int quantum;
    estado_proceso estado;
    registros_cpu registros;
}PCB;

typedef enum {
    INST_EXIT,
    INST_WAIT,
    INST_SIGNAL,
    SOLICITUD_IO
}motivo_dispatch;

typedef struct {
    PCB* proceso;
    motivo_dispatch motivo;
}t_dispatch;

#endif /* PCB_H */