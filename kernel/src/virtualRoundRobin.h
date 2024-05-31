#ifndef VRR_H
#define VRR_H

#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <commons/temporal.h>
#include <commons/collections/queue.h>
#include <utils/pcb.h>
#include <roundRobin.h>

extern t_queue* colaPrioritaria;
extern pthread_mutex_t mutexColaPrioritaria;

#endif /* VRR_H */