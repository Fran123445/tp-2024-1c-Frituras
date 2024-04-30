#include <planificacion.h>
#include <unistd.h>

void esperarQuantumCompleto(int);
void esperarFinQuantum(int);
void enviarProcesoACPU_RR(PCB*);
void enviarInterrupcion();