#ifndef STDIN_OUT_H
#define STDIN_OUT_H

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

void administrarSTDIN(int* socket_cliente);
void administrarSTDOUT(int* socket_cliente);

typedef struct {
    uint32_t direccion;
    uint32_t tamanio;
} t_direccionMemoria;

typedef struct {
    PCB* proceso;
    t_list* direcciones;
} t_solicitudIOSTDIN_OUT;

t_solicitudIOSTDIN_OUT* solicitudIOSTDIN_OUT_create(PCB* proceso, t_buffer* buffer);

#endif