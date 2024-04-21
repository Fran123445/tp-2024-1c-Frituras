#ifndef PROC_H
#define PROC_H

#include <commons/collections/list.h>

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
    // Falta registrosCPU
}PCB;

extern int ultimoPIDAsignado;
extern t_list* colaNew;
extern t_list* colaReady;
extern t_list* colaBlocked;
extern t_list* colaExit;

#endif /* PROC_H */