#ifndef CONN_H
#define CONN_H

#include <stdlib.h>
#include <stdio.h>
#include <threads.h>
#include <semaphore.h>
#include <utils/server.h>
#include <utils/serializacion.h>
#include <commons/collections/queue.h>
#include <commons/log.h>

extern t_log* logger;
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

void esperarClientesIO(t_conexion_escucha* params);
void administrarInterfazGenerica(int socket_cliente);

#endif /* CONN_H */