#ifndef VRR_H
#define VRR_H

#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <commons/temporal.h>
#include <commons/collections/queue.h>
#include <utils/pcb.h>
#include <planificacion/roundRobin.h>

extern int cpuLibre;
extern bool procesoInterrumpido;

typedef enum {
    COLA_READY,
    COLA_PRIORITARIA
} colaProveniente;

#endif /* VRR_H */