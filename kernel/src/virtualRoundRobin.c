#include <virtualRoundRobin.h>

// indica que si el ultimo proceso enviado salio o no de la cola prioritaria
int ultimoPrioritario = 0;

int pidProcesoEnEjecucion;

pthread_mutex_t mutexColaPrioritaria;
t_queue* colaPrioritaria;
sem_t procesosEnColaPrioritaria;
t_temporal* tiempoTranscurrido;

void enviarAColaPrioritaria(PCB* proceso) {
    pthread_mutex_lock(&mutexColaPrioritaria);
    cambiarEstado(proceso, ESTADO_READY);
    queue_push(colaPrioritaria, proceso);
    pthread_mutex_unlock(&mutexColaPrioritaria);

    if (!ultimoPrioritario) {
        enviarInterrupcion(pidProcesoEnEjecucion);
        ultimoPrioritario = 1;
    }

    sem_post(&procesosEnColaPrioritaria);
}

PCB* sacarSiguienteDeColaPrioritaria() {
    sem_wait(&procesosEnColaPrioritaria);

    pthread_mutex_lock(&mutexColaPrioritaria);
    PCB* proceso = queue_pop(colaPrioritaria);
    cambiarEstado(proceso, ESTADO_EXEC);
    pthread_mutex_unlock(&mutexColaPrioritaria);

    return proceso;
}

colaProveniente seleccionarSiguiente(PCB* proceso) {
    colaProveniente cola;
    if (!queue_is_empty(colaPrioritaria)) {
        proceso = sacarSiguienteDeColaPrioritaria();
        cola = COLA_PRIORITARIA;
    } else {
        proceso = sacarSiguienteDeReady();
        cola = COLA_READY;
    }
    
    return cola;
}

void ejecutarSiguienteVRR() {
    // siento que me quedaron medio engorrosas esta funcion y la de seleccionar siguiente,
    // pero en este momento no se me ocurre una mejor manera de plantearlas
    PCB* proceso;
    while(1) {
        sem_wait(&cpuDisponible);
        colaProveniente cola = seleccionarSiguiente(proceso);
        enviarProcesoACPU_RR(proceso);
        pidProcesoEnEjecucion = proceso->PID;
        ultimoPrioritario = (cola == COLA_PRIORITARIA) ? 1 : 0;
        tiempoTranscurrido = temporal_create();
    }
}

void asignarQuantum(PCB* proceso) {
    sem_post(&finalizarQuantum);
    temporal_stop(tiempoTranscurrido);

    if (!procesoInterrumpido) {
        proceso->quantum = quantumInicial;
    } else {
        proceso->quantum -= temporal_gettime(tiempoTranscurrido);
    }
}

void planificarPorVRR(op_code operacion, PCB* proceso, t_buffer* buffer) {
    int cpuLibre;
    switch (operacion) {
        case ENVIAR_IO_GEN_SLEEP:
            asignarQuantum(proceso);
            enviarAIOGenerica(proceso, operacion, buffer);
            break;
        case OPERACION_FINALIZADA:
            if (proceso->quantum == quantumInicial) {
                enviarAReady(proceso);
            } else {
                enviarAColaPrioritaria(proceso);
            }
            return;
        case INSTRUCCION_WAIT:
            cpuLibre = instruccionWait(proceso, buffer);
            if (cpuLibre) { 
                sem_post(&finalizarQuantum);
                break; 
            } else return;
        case INSTRUCCION_SIGNAL:
            cpuLibre = instruccionSignal(proceso, buffer);
            if (cpuLibre) { 
                sem_post(&finalizarQuantum);
                break; 
            } else return;
        case INSTRUCCION_EXIT:
            sem_post(&finalizarQuantum);
            enviarAExit(proceso, SUCCESS);
            break;
        default:
            pthread_mutex_lock(&mutexLogger);
            log_error(logger, "Instruccion no válida");
            pthread_mutex_unlock(&mutexLogger);
            break;
    }

    temporal_destroy(tiempoTranscurrido);
    sem_post(&cpuDisponible);
}