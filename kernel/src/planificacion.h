#ifndef PLAN_H
#define PLAN_H

#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <string.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <utils/serializacion.h>
#include <planificadorLP.h>
#include <interfaces.h>
#include <recursos.h>

extern pthread_t pth_colaExit;
extern pthread_t pth_colaNew;
extern pthread_t pth_colaReady;
extern pthread_t pth_recibirProc;

extern int socketCPUDispatch;
extern int socketCPUInterrupt;

extern sem_t llegadaProceso;

extern pthread_mutex_t mutexLogger;

void inicializarSemaforosYMutex();

// Espera a que lleguen procesos a la cola NEW para pasarlos a READY si el grado de multiprogramacion lo permite
void procesoNewAReady();

// Saca el primer proceso de Ready y lo cambia a estado EXEC
PCB* sacarSiguienteDeReady();

void enviarProcesoACPU(PCB*);

// Espera a que la CPU se encuentre disponible y le envia el siguiente proceso en READY para que lo ejecute
void ejecutarSiguiente();

// Recibe un t_dispatch y planifica en funcion de lo que solicita
void planificarRecibido(op_code operacion, PCB* proceso, t_buffer* buffer);

// Lee el t_dispatch del buffer y llama a planificarRecibido
void leerBufferYPlanificar(op_code operacion);

// Envia el proceso pasado por parametro a la IO generica en el buffer
void enviarAIOGenerica(PCB* proceso, op_code operacion, t_buffer* buffer);

// Realiza WAIT sobre un recurso y se lo asigna al proceso pasado por parametro
// Devuelve 0 si el proceso fue devuelto a la CPU y 1 si el proceso fue bloqueado o enviado a exit
int instruccionWait(PCB* proceso, t_buffer* buffer);

// Realiza SIGNAL sobre un recurso y se lo asigna al proceso pasado por parametro
// Devuelve 0 si el proceso fue devuelto a la CPU y 1 si el proceso fue enviado a exit
int instruccionSignal(PCB* proceso, t_buffer* buffer);

// Inicia los hilos para la planificacion por FIFO
void planificacionPorFIFO();

#endif /* PLAN_H */