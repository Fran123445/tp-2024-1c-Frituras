#include <roundRobin.h>

bool procesoInterrumpido;

void enviarInterrupcion() {
    //send
}

void esperarQuantumCompleto(int quantum) {
    sleep(quantum);
    enviarInterrupcion();

    procesoInterrumpido = 1;
}

void esperarFinQuantum(int quantum) {
    pthread_t esperaQuantum;
    pthread_create(&esperaQuantum,
						NULL,
						(void*) esperarQuantumCompleto,
						&quantum);      /* El & es porque si no me tira un warning por un tema de
                                        punteros. No se si esta bien o si cuando lo probemos va
                                        a hacer que reviente todo */

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