#ifndef IO_GEN_H
#define IO_GEN_H

#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <commons/collections/queue.h>
#include <commons/log.h>
#include <utils/serializacion.h>
#include <interfaces/interfaces.h>

extern void planificar(op_code, PCB*, t_buffer*);
extern t_log* logger;

extern pthread_mutex_t mutexPlanificador;

typedef struct {
    PCB* proceso;
    int unidadesTrabajo;
} t_solicitudIOGenerica;

void administrarInterfazGenerica(int* socket_cliente);
t_solicitudIOGenerica* solicitudIOGenerica_create(PCB* proceso, t_buffer* buffer);

#endif