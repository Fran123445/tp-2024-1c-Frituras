#ifndef ESTRUCTURAS_PROC_H
#define ESTRUCTURAS_PROC_H

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

#endif /* ESTRUCTURAS_PROC_H */