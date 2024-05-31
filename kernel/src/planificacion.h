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

// Envia un proceso a la cola exit con su respectivo motivo
void enviarAExit(PCB* pcb, motivo_exit motivo);

// Cambia el estado de un proceso y loggea el estado anterior y el nuevo
void cambiarEstado(PCB* proceso, estado_proceso estado);

// Espera a que lleguen PCBs a exit para eliminarlos y liberar memoria
void vaciarExit();

// Espera a que lleguen procesos a la cola NEW para pasarlos a READY si el grado de multiprogramacion lo permite
void procesoNewAReady();

// Saca el primer proceso de Ready y lo cambia a estado EXEC
PCB* sacarSiguienteDeReady();

void enviarProcesoACPU(PCB*);

// Espera a que la CPU se encuentre disponible y le envia el siguiente proceso en READY para que lo ejecute
void ejecutarSiguiente();

// Recibe un t_dispatch y planifica en funcion de lo que solicita
void planificarRecibido(op_code operacion, t_buffer* buffer);

// Lee el t_dispatch del buffer y llama a planificarRecibido
void leerBufferYPlanificar(op_code operacion);

// Inicia los hilos para la planificacion por FIFO
void planificacionPorFIFO();

// Setea el flag finalizar en 1 y hace un signal para cada semaforo
void finalizarHilos();

#endif /* PLAN_H */