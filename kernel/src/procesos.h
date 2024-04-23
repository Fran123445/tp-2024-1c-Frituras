#ifndef PROC_H
#define PROC_H

#include <stdlib.h>
#include <stdio.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <stdint.h>
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

extern t_list* listadoProcesos;

void iniciarProceso(char*);
void finalizarProceso(int);
PCB* hallarPCB(int);
t_list* enumEstadoACola(int);

#endif /* PROC_H */