#include <planificacion/roundRobin.h>

bool procesoInterrumpido;

void esperarQuantumCompleto(PCB* proceso) {
    int pid = proceso->PID;
    usleep(proceso->quantum*1000);
    enviarInterrupcion(pid, FIN_DE_Q);

    procesoInterrumpido = 1;
}

void esperarVuelta(PCB* proceso) {
    procesoInterrumpido = 0;
    pthread_t esperaQuantum;
    pthread_create(&esperaQuantum,
						NULL,
						(void*) esperarQuantumCompleto,
						proceso);

    sem_wait(&finalizarQuantum);

    // esto es por si el proceso termina o no de ejecutarse antes que se cumpla el quantum
    if (procesoInterrumpido) {
        pthread_join(esperaQuantum, NULL);
        log_info(logger, "Proceso %d interrumpido por fin de quantum", proceso->PID);
        sem_post(&quantumFinalizado);
        return;
    }

    pthread_cancel(esperaQuantum);
    sem_post(&quantumFinalizado);
}

void cortarQuantum() {
    sem_post(&finalizarQuantum);
    sem_wait(&quantumFinalizado);
}

void enviarProcesoACPU_RR(PCB* proceso) {
    enviarProcesoACPU(proceso);
    pthread_t quantum;
    pthread_create(&quantum,
						NULL,
						(void*) esperarVuelta,
						proceso);
    pthread_detach(quantum);
}

void enviarAIORR(PCB* proceso, op_code operacion, t_buffer* buffer) {
    cortarQuantum();
    enviarAIO(proceso, operacion, buffer);
    cpuLibre = 1;
}

void waitRR(PCB* proceso, t_buffer* buffer) {
    cpuLibre = instruccionWait(proceso, buffer);
    if (cpuLibre) { 
        sem_post(&finalizarQuantum);
    }
}

void signalRR(PCB* proceso, t_buffer* buffer) {
    cortarQuantum();
    enviarAExit(proceso, SUCCESS);
    cpuLibre = 1;
}

void interrupcionRR(PCB* proceso) {
    cortarQuantum();
    enviarAReady(proceso);
    cpuLibre = 1;
}

void criterioEnvioRR() {
    if (cpuLibre && !queue_is_empty(colaReady)) {
        enviarProcesoACPU_RR(sacarSiguienteDeReady());
        cpuLibre = 0;
    }
}

void exitRR(PCB* proceso) {
    cortarQuantum();
    exitFIFO(proceso);
}

void setRR() {
    instIO = enviarAIORR;
    IOFinalizada = operacionFinalizadaFIFO;
    instWait = waitRR;
    instSignal = signalRR;
    instExit = exitRR;
    interrupcion = interrupcionRR;
    criterioEnvio = criterioEnvioRR;
}