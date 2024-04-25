#ifndef PROC_H
#define PROC_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <planificacion.h>

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
    int estado;
    // Falta registrosCPU
}PCB;

extern int siguientePID;

void iniciarProceso(char*);
void finalizarProceso(int);
PCB* hallarPCB(int);
t_queue* enumEstadoACola(int);
void sacarProceso(t_queue*, PCB*);

#endif /* PROC_H */