#ifndef DIALFS_H
#define DIALFS_H

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
    uint32_t direccion;
    uint32_t tamanio;
    uint32_t ubicacionPuntero;
} t_infoArchivo;
typedef struct {
    PCB* proceso;
    op_code operacion;
    char* nombreArchivo;
    int tamanio;
    t_list* direcciones;
} t_solicitudDIALFS;

void administrarDIALFS(int* socket_cliente);

t_solicitudDIALFS* solicitudDIALFS_create(PCB* proceso, op_code operacion, t_buffer* buffer);

#endif