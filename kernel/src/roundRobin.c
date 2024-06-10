#include <roundRobin.h>

bool procesoInterrumpido;

void enviarInterrupcion(int PID) {
    t_paquete* paquete = crear_paquete(INTERRUPCION);
    agregar_int_a_paquete(paquete, PID);
    enviar_paquete(paquete, socketCPUInterrupt);
    eliminar_paquete(paquete);
}

void esperarQuantumCompleto(PCB* proceso) {
    usleep(proceso->quantum*1000);
    enviarInterrupcion(proceso->PID);

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
        return;
    }

    pthread_cancel(esperaQuantum);
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

// Seguramente refactorice esto para tener un solo switch en vez de uno en cada uno
void planificarPorRR(op_code operacion, PCB* proceso, t_buffer* buffer) {
    switch (operacion) {
        case CREACION_PROCESO:
            pthread_mutex_lock(&mutexNew);
            PCB* nuevoProceso = queue_pop(colaNew);
            enviarAReady(nuevoProceso);
            pthread_mutex_unlock(&mutexNew);
            break;
        case ENVIAR_IO_GEN_SLEEP:
            sem_post(&finalizarQuantum);
            enviarAIOGenerica(proceso, operacion, buffer);
            cpuLibre = 1;
            break;
        case OPERACION_FINALIZADA:
            enviarAReady(proceso);
            break;
        case INSTRUCCION_WAIT:
            cpuLibre = instruccionWait(proceso, buffer);
            if (cpuLibre) { 
                sem_post(&finalizarQuantum);
            }
            break;
        case INSTRUCCION_SIGNAL:
            cpuLibre = instruccionSignal(proceso, buffer);
            if (cpuLibre) { 
                sem_post(&finalizarQuantum);
            }
            break;
        case INSTRUCCION_EXIT:
            sem_post(&finalizarQuantum);
            enviarAExit(proceso, SUCCESS);
            cpuLibre = 1;
            break;
        case INTERRUPCION:
            enviarAReady(proceso);
            cpuLibre = 1;
            break;
        default:
            pthread_mutex_lock(&mutexLogger);
            log_error(logger, "Instruccion no válida");
            pthread_mutex_unlock(&mutexLogger);
            break;
    }


    if (cpuLibre && !queue_is_empty(colaReady)) {
        enviarProcesoACPU_RR(sacarSiguienteDeReady());
        cpuLibre = 0;
    }
}

void iniciarRR() {
    pthread_create(&pth_colaExit,
						NULL,
						(void*) vaciarExit,
						NULL);
    pthread_create(&pth_colaNew,
						NULL,
						(void*) procesoNewAReady,
						NULL);
    pthread_create(&pth_recibirProc,
						NULL,
						(void*) recibirDeCPU,
						NULL);
}