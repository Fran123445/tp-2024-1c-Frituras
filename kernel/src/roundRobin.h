#include <planificacion.h>
#include <unistd.h>

// Duerme el hilo hasta pasado el tiempo correspondiente al quantum
void esperarQuantumCompleto(int*);

// Espera a que el proceso vuelva por fin de quantum o porque termino de ejecutarse
void esperarFinQuantum(int*);

// Envia el proceso a CPU e inicia el conteo del quantum
void enviarProcesoACPU_RR(PCB*);
void enviarInterrupcion();