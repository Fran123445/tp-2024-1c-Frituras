#ifndef PLANLP_H
#define PLANLP_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/log.h>
#include <commons/string.h>
#include <utils/serializacion.h>
#include <utils/pcb.h>
#include <interfaces.h>
typedef enum {
    SUCCESS,
    INVALID_RESOURCE,
    INVALID_INTERFACE,
    OOM,
    INTERRUPTED_BY_USER
} motivo_exit;
typedef struct {
    PCB* pcb;
    motivo_exit motivo;
} procesoEnExit;

extern t_log* logger;

extern int socketMemoria;;

extern int quantumInicial;

extern sem_t procesosEnNew;
extern sem_t procesosEnExit;
extern sem_t gradoMultiprogramacion;
extern pthread_mutex_t mutexNew;
extern pthread_mutex_t mutexReady;
extern pthread_mutex_t mutexExit;
extern pthread_mutex_t mutexListaProcesos;
extern pthread_mutex_t mutexListaInterfaces;

extern t_queue* colaNew;
extern t_queue* colaReady;
extern t_queue* colaExit;
extern t_list* interfacesConectadas;
extern t_list* listadoProcesos;

extern int siguientePID;

void logProcesosEnCola(estado_proceso estado, char* nombreCola, t_queue* cola);

// Espera a que lleguen PCBs a exit para eliminarlos y liberar memoria
void vaciarExit();

// Envia un proceso a la cola Exit con su respectivo motivo
void enviarAExit(PCB* pcb, motivo_exit motivo);

// Cambia el estado de un proceso y loggea el estado anterior y el nuevo
void cambiarEstado(PCB* proceso, estado_proceso estado);

// Envia un proceso a la cola Ready
void enviarAReady(PCB* pcb);

// Devuelve un string con el nombre del dato enumerado 
char* enumEstadoAString(estado_proceso);

// Crea un nuevo PCB y lo envia a la cola NEW
void iniciarProceso(char*);

// Envia a EXIT el proceso con el PID enviado por parametro
void finalizarProceso(int);

// Busca un PCB en base a un PID
PCB* hallarPCB(int);

// Devuelve la cola correspondiente al estado enviado
t_queue* enumEstadoACola(int);

// Saca el PCB enviado por parametro de la cola enviada por parametro.
void sacarProceso(t_queue*, PCB*);

#endif /* PLANLP_H */