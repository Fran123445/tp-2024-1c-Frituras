#ifndef CONN_H
#define CONN_H

#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <commons/collections/queue.h>
#include <commons/log.h>
#include <utils/server.h>
#include <utils/serializacion.h>
#include <planificadorLP.h>

extern void planificar(op_code, PCB*, t_buffer*);
extern t_log* logger;

extern pthread_mutex_t mutexPlanificador;

typedef enum {
    INTERFAZ_GENERICA,
    INTERFAZ_STDIN,
    INTERFAZ_STDOUT
} tipoInterfaz;

typedef struct {
    char* nombreInterfaz;
    tipoInterfaz tipo;
    t_queue* procesosBloqueados;
    pthread_mutex_t mutex;
    sem_t semaforo;
} t_IOConectada;

typedef struct {
    PCB* proceso;
    int unidadesTrabajo;
} t_solicitudIOGenerica;

void esperarClientesIO(t_conexion_escucha* params);
void administrarInterfazGenerica(int* socket_cliente);
t_IOConectada* hallarInterfazConectada(char* nombre);
bool comprobarOperacionValida(t_IOConectada* interfaz, op_code operacion);
t_solicitudIOGenerica* solicitudIOGenerica_create(PCB* proceso, t_buffer* buffer);

#endif /* CONN_H */