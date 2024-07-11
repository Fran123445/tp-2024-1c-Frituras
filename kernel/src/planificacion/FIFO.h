#ifndef FIFO_H
#define FIFO_H

#include <stdlib.h>
#include <stdio.h>
#include <planificacion/planificacion.h>


extern void (*instIO)(PCB*, op_code, t_buffer*);
extern void (*IOFinalizada)(PCB*);
extern void (*instWait)(PCB*, t_buffer*);
extern void (*instSignal)(PCB*, t_buffer*);
extern void (*instExit)(PCB*);
extern void (*interrupcion)(PCB*);
extern void (*criterioEnvio)();

extern int cpuLibre;

void enviarAIOFIFO(PCB* proceso, op_code operacion, t_buffer* buffer);

void operacionFinalizadaFIFO(PCB* proceso);

void waitFIFO(PCB* proceso, t_buffer* buffer);

void signalFIFO(PCB* proceso, t_buffer* buffer);

void exitFIFO(PCB* proceso);

void criterioEnvioFIFO();

void setFIFO();

#endif /* RR_H */