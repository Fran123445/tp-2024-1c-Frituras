#ifndef VRR_H
#define VRR_H

#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <commons/temporal.h>
#include <commons/collections/queue.h>
#include <utils/pcb.h>
#include <roundRobin.h>

extern sem_t cpuDisponible;

typedef enum {
    COLA_READY,
    COLA_PRIORITARIA
} colaProveniente;

#endif /* VRR_H */