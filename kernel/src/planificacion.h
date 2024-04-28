#ifndef PLAN_H
#define PLAN_H

#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <string.h>
#include <procesos.h>

void inicializarSemaforosYMutex();
void vaciarExit();
void procesoNewAReady();
void ejecutarSiguiente();
void enviarProcesoACPU(PCB*);
void planificarRecibidoPorFIFO(t_dispatch*); /* esta funcion puede o no que cambie. Depende de como plantee RR */


#endif /* PLAN_H */