#include <planificacion/virtualRoundRobin.h>

// indica que si el ultimo proceso enviado salio o no de la cola prioritaria
int ultimoPrioritario = 0;

int pidProcesoEnEjecucion;

pthread_mutex_t mutexColaPrioritaria;
t_queue* colaPrioritaria;
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
}

PCB* sacarSiguienteDeColaPrioritaria() {
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
    switch (operacion) {
        case CREACION_PROCESO:
            pthread_mutex_lock(&mutexNew);
            PCB* proceso = queue_pop(colaNew);
            enviarAReady(proceso);
            pthread_mutex_unlock(&mutexNew);
            break;
        case ENVIAR_IO_GEN_SLEEP:
            temporal_destroy(tiempoTranscurrido);
            asignarQuantum(proceso);
            enviarAIOGenerica(proceso, operacion, buffer);
            cpuLibre = 0;
            break;
        case OPERACION_FINALIZADA:
            if (proceso->quantum == quantumInicial) {
                enviarAReady(proceso);
            } else {
                enviarAColaPrioritaria(proceso);
            }
            break;
        case INSTRUCCION_WAIT:
            cpuLibre = instruccionWait(proceso, buffer);
            if (cpuLibre) { 
                sem_post(&finalizarQuantum);
            }
            break;
                break; 
            break;
        case INSTRUCCION_SIGNAL:
            cpuLibre = instruccionSignal(proceso, buffer);
            if (cpuLibre) { 
                sem_post(&finalizarQuantum);
            } 
            break;
                break; 
            break;
        case INSTRUCCION_EXIT:
            sem_post(&finalizarQuantum);
            enviarAExit(proceso, SUCCESS);
            cpuLibre = 0;
            break;
        default:
            pthread_mutex_lock(&mutexLogger);
            log_error(logger, "Instruccion no válida");
            pthread_mutex_unlock(&mutexLogger);
            break;
    }

    if (cpuLibre && (!queue_is_empty(colaReady) || !queue_is_empty(colaPrioritaria))) {
        PCB* procesoAEnviar;
        colaProveniente cola = seleccionarSiguiente(procesoAEnviar);
        enviarProcesoACPU_RR(procesoAEnviar);
        pidProcesoEnEjecucion = proceso->PID;
        ultimoPrioritario = (cola == COLA_PRIORITARIA) ? 1 : 0;
        cpuLibre = 0;
        tiempoTranscurrido = temporal_create();
    }
}