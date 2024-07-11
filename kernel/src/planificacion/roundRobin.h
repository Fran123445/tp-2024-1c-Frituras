#ifndef RR_H
#define RR_H

#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <unistd.h>
#include <planificacion/planificacion.h>
#include <planificacion/FIFO.h>

extern int cpuLibre;
extern sem_t quantumFinalizado;

void cortarQuantum();

// Duerme el hilo hasta pasado el tiempo correspondiente al quantum
void esperarQuantumCompleto(PCB* proceso);

// Espera a que el proceso vuelva por fin de quantum o porque termino de ejecutarse
void esperarVuelta(PCB* proceso);

// Envia el proceso a CPU e inicia el conteo del quantum
void enviarProcesoACPU_RR(PCB*);

void enviarAIORR(PCB* proceso, op_code operacion, t_buffer* buffer);

void waitRR(PCB* proceso, t_buffer* buffer);

void signalRR(PCB* proceso, t_buffer* buffer);

void interrupcionRR(PCB* proceso);

void exitRR();

void criterioEnvioRR();

void setRR();

#endif /* RR_H */