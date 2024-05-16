#include <planificacion.h>
#include <semaphore.h>
#include <unistd.h>

extern sem_t cpuDisponible;

// Envia una interrupcion por fin de quantum
void enviarInterrupcion(int PID);

// Duerme el hilo hasta pasado el tiempo correspondiente al quantum
void esperarQuantumCompleto(PCB* proceso);

// Espera a que el proceso vuelva por fin de quantum o porque termino de ejecutarse
void esperarFinQuantum(PCB* proceso);

// Envia el proceso a CPU e inicia el conteo del quantum
void enviarProcesoACPU_RR(PCB*);

// Envia una interrupcion a la CPU
void enviarInterrupcion();

void planificacionPorRR();