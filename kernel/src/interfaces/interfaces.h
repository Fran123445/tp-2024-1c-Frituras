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

extern void administrarDIALFS(int*);
extern void administrarInterfazGenerica(int*);
extern void administrarSTDIN(int*);
extern void administrarSTDOUT(int*);

typedef enum {
    INTERFAZ_GENERICA,
    INTERFAZ_STDIN,
    INTERFAZ_STDOUT,
    INTERFAZ_DIALFS
} tipoInterfaz;

typedef struct {
    char* nombreInterfaz;
    tipoInterfaz tipo;
    t_queue* procesosBloqueados;
    pthread_mutex_t mutex;
    sem_t semaforo;
} t_IOConectada;

void esperarClientesIO(t_conexion_escucha* params);
t_IOConectada* IOConectado_create(int socket_cliente, tipoInterfaz tipo);
t_IOConectada* hallarInterfazConectada(char* nombre);
bool comprobarOperacionValida(t_IOConectada* interfaz, op_code operacion);
void liberarInterfazConectada(t_IOConectada* interfaz);

#endif /* CONN_H */