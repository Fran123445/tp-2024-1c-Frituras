#include <roundRobin.h>

bool procesoInterrumpido;

void enviarInterrupcion() {
    t_paquete* paquete = crear_paquete();
    paquete->codigo_operacion = INTERRUPCION;
    enviar_paquete(paquete, socketCPUInterrupt);
    eliminar_paquete(paquete);
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
						&(proceso->quantum));
    pthread_detach(quantum);
}

void ejercutarSiguienteRR() {
    while(1) {
        sem_wait(&cpuDisponible);
        PCB* proceso = sacarSiguienteDeReady();    
        enviarProcesoACPU_RR(proceso);
    }
}