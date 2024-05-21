#include <roundRobin.h>

bool procesoInterrumpido;

void enviarInterrupcion(int PID) {
    t_paquete* paquete = crear_paquete(INTERRUPCION);
    agregar_int_a_paquete(paquete, PID);
    enviar_paquete(paquete, socketCPUInterrupt);
    eliminar_paquete(paquete);
}

void esperarQuantumCompleto(PCB* proceso) {
    sleep(proceso->quantum);
    enviarInterrupcion(proceso->PID);

    procesoInterrumpido = 1;
}

void esperarFinQuantum(PCB* proceso) {
    pthread_t esperaQuantum;
    pthread_create(&esperaQuantum,
						NULL,
						(void*) esperarQuantumCompleto,
						proceso);

    sem_wait(&llegadaProceso);

    // esto es por si el proceso termina o no de ejecutarse antes que se cumpla el quantum
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
						proceso);
    pthread_detach(quantum);
}

void ejecutarSiguienteRR() {
    while(1) {
        sem_wait(&cpuDisponible);
        PCB* proceso = sacarSiguienteDeReady();    
        enviarProcesoACPU_RR(proceso);
    }
}

void recibirDeCPURR() {
    while(1) {
        recibir_operacion(socketCPUDispatch);
        sem_post(&llegadaProceso);
        leerBufferYPlanificar();
    }
}

void planificacionPorRR() {
    pthread_create(&pth_colaExit,
						NULL,
						(void*) vaciarExit,
						NULL);
    pthread_detach(pth_colaExit);

    pthread_create(&pth_colaNew,
						NULL,
						(void*) procesoNewAReady,
						NULL);
    pthread_detach(pth_colaNew);

    pthread_create(&pth_colaReady,
						NULL,
						(void*) ejecutarSiguienteRR,
						NULL);
    pthread_detach(pth_colaReady);

    pthread_create(&pth_recibirProc,
						NULL,
						(void*) recibirDeCPURR,
						NULL);
    pthread_detach(pth_recibirProc);
}