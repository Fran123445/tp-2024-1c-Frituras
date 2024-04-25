#ifndef PLAN_H
#define PLAN_H

#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/log.h>
#include <procesos.h>

extern int gradoMultiprogramacion; // esto va a pasar a ser un semaforo casi seguro
extern t_log* logger; // no estoy seguro de que esto se vaya a quedar aca

extern sem_t procesosEnExit;
extern pthread_mutex_t mutexExit;
extern pthread_mutex_t mutexListaProcesos;

extern t_queue* colaNew;
extern t_queue* colaReady;
extern t_queue* colaBlocked;
extern t_queue* colaExit;
extern t_list* listadoProcesos;

void inicializarSemaforosYMutex();
void vaciarExit();

#endif /* PLAN_H */