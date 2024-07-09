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

typedef struct {
    PCB* proceso;
    int unidadesTrabajo;
} t_solicitudIOGenerica;

typedef struct {
    uint32_t direccion;
    uint32_t tamanio;
} t_direccionMemoria;

typedef struct {
    PCB* proceso;
    t_list* direcciones;
} t_solicitudIOSTDIN_OUT;

typedef struct {
    PCB* proceso;
    op_code operacion;
    char* nombreArchivo;
    int direccion;
    int tamanio;
    int ubicacionPuntero;
} t_solicitudDIALFS;

void esperarClientesIO(t_conexion_escucha* params);
void administrarInterfazGenerica(int* socket_cliente);
void administrarSTDIN(int* socket_cliente);
void administrarSTDOUT(int* socket_cliente);
t_IOConectada* hallarInterfazConectada(char* nombre);
bool comprobarOperacionValida(t_IOConectada* interfaz, op_code operacion);
t_solicitudIOGenerica* solicitudIOGenerica_create(PCB* proceso, t_buffer* buffer);
t_solicitudIOSTDIN_OUT* solicitudIOSTDIN_OUT_create(PCB* proceso, t_buffer* buffer);

#endif /* CONN_H */