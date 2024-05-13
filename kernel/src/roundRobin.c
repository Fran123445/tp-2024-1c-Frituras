#include <roundRobin.h>

bool procesoInterrumpido;

void enviarInterrupcion() {
    //send
}

void esperarQuantumCompleto(int* quantum) {
    sleep(*quantum);
    enviarInterrupcion();

    procesoInterrumpido = 1;
}

void esperarFinQuantum(int* quantum) {
    pthread_t esperaQuantum;
    pthread_create(&esperaQuantum,
						NULL,
						(void*) esperarQuantumCompleto,
						quantum);

    sem_wait(&llegadaProceso);

    if (procesoInterrumpido) {
        pthread_join(esperaQuantum, NULL);
        return;
    }

    pthread_cancel(esperaQuantum);
    procesoInterrumpido = 0;
}

void enviarProcesoACPU_RR(PCB* proceso) {
    enviarProcesoACPU(proceso);
    pthread_t quantum;
    pthread_create(&quantum,
						NULL,
						(void*) esperarFinQuantum,
						&proceso->quantum); // lo mismo de arriba
    pthread_detach(quantum);
}