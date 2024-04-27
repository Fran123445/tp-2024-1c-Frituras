#ifndef PLAN_H
#define PLAN_H

#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/log.h>
#include <commons/string.h>
#include <string.h>
#include <procesos.h>

extern t_log* logger; // no estoy seguro de que esto se vaya a quedar aca

extern sem_t procesosEnNew;
extern sem_t procesosEnExit;
extern sem_t gradoMultiprogramacion;
extern pthread_mutex_t mutexExit;
extern pthread_mutex_t mutexListaProcesos;
extern pthread_mutex_t mutexNew;
extern pthread_mutex_t mutexReady;
extern pthread_mutex_t mutexBlocked;

extern t_queue* colaNew;
extern t_queue* colaReady;
extern t_queue* colaBlocked;
extern t_queue* colaExit;
extern t_list* listadoProcesos;

void inicializarSemaforosYMutex();
void vaciarExit();
void procesoNewAReady();

#endif /* PLAN_H */