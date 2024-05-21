#ifndef CONN_H
#define CONN_H

#include <stdlib.h>
#include <stdio.h>
#include <threads.h>
#include <semaphore.h>
#include <commons/collections/queue.h>
#include <commons/log.h>
#include <utils/server.h>
#include <utils/serializacion.h>
#include <procesos.h>
#include <planificacion.h>

extern t_log* logger;

extern pthread_mutex_t mutexLogger;
typedef enum {
    INTERFAZ_GENERICA,
    INTERFAZ_STDIN,
    INTERFAZ_STDOUT
} tipoInterfaz;

typedef struct {
    char* nombreInterfaz;
    tipoInterfaz tipo;
    t_queue* cola;
    pthread_mutex_t mutex;
    sem_t semaforo;
} t_IOConectado;

typedef struct {
    PCB* proceso;
    int unidadesTrabajo;
} t_solicitudIOGenerica;

void esperarClientesIO(t_conexion_escucha* params);
void administrarInterfazGenerica(int* socket_cliente);
t_IOConectado* hallarInterfazConectada(char* nombre);
bool comprobarOperacionValida(t_IOConectado* interfaz, t_tipoInstruccion inst);

#endif /* CONN_H */