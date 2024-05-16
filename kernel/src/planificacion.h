#ifndef PLAN_H
#define PLAN_H

#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <string.h>
#include <procesos.h>

extern int socketCPUDispatch;
extern int socketCPUInterrupt;

extern sem_t llegadaProceso;

void inicializarSemaforosYMutex();

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
void planificarRecibido(t_dispatch* dispatch);

// Inicia los hilos para la planificacion por FIFO
void planificacionPorFIFO();

#endif /* PLAN_H */