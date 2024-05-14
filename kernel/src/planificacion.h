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

// Espera a que la CPU se encuentre disponible y le envia el siguiente proceso en READY para que lo ejecute
void ejecutarSiguiente();
void enviarProcesoACPU(PCB*);
void planificarRecibidoPorFIFO(t_dispatch*); /* esta funcion puede o no que cambie. Depende de como plantee RR */


#endif /* PLAN_H */