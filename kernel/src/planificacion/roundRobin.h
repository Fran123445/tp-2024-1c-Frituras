#ifndef RR_H
#define RR_H

#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>
#include <planificacion/planificacion.h>

extern int cpuLibre;

// Envia una interrupcion por fin de quantum
void enviarInterrupcion(int PID);

// Duerme el hilo hasta pasado el tiempo correspondiente al quantum
void esperarQuantumCompleto(PCB* proceso);

// Espera a que el proceso vuelva por fin de quantum o porque termino de ejecutarse
void esperarVuelta(PCB* proceso);

// Envia el proceso a CPU e inicia el conteo del quantum
void enviarProcesoACPU_RR(PCB*);

// Envia una interrupcion a la CPU
void enviarInterrupcion();

void ejecutarSiguienteRR();

void planificarPorRR(op_code operacion, PCB* proceso, t_buffer* buffer);

void iniciarRR();

#endif /* RR_H */