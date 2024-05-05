#ifndef PCB_H
#define PCB_H

#include <stdlib.h>
#include <stdio.h>

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
    // Falta registrosCPU
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
}t_dispatch; //despues hay que moverlo a utils para que la CPU tambien acceda a la estructura esta (el enum lo mismo)

#endif /* PCB_H */