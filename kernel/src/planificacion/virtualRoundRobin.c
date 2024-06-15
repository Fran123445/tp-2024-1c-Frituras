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
    logProcesosEnCola(ESTADO_READY, "READY+", colaPrioritaria);
    pthread_mutex_unlock(&mutexColaPrioritaria);

    if (!ultimoPrioritario) {
        enviarInterrupcion(pidProcesoEnEjecucion, FIN_DE_Q); // habria que o crear un opcode nuevo o cambiarle el nombre a ese
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

colaProveniente seleccionarSiguiente(PCB** proceso) {
    colaProveniente cola;
    if (!queue_is_empty(colaPrioritaria)) {
        *proceso = sacarSiguienteDeColaPrioritaria();
        cola = COLA_PRIORITARIA;
    } else {
        *proceso = sacarSiguienteDeReady();
        cola = COLA_READY;
    }
    
    return cola;
}

void asignarQuantum(PCB* proceso) {
    cortarQuantum();
    temporal_stop(tiempoTranscurrido);

    if (!procesoInterrumpido) {
        proceso->quantum = quantumInicial;
    } else {
        proceso->quantum -= temporal_gettime(tiempoTranscurrido);
    }
}

void enviarAIOGenericaVRR(PCB* proceso, op_code operacion, t_buffer* buffer) {
    temporal_destroy(tiempoTranscurrido);
    asignarQuantum(proceso);
    enviarAIOGenerica(proceso, operacion, buffer);
    cpuLibre = 1;
}

void operacionFinalizadaVRR(PCB* proceso) {
    if (proceso->quantum == quantumInicial) {
        enviarAReady(proceso);
    } else {
        enviarAColaPrioritaria(proceso);
    }
}

void criterioEnvioVRR() {
    if (cpuLibre && (!queue_is_empty(colaReady) || !queue_is_empty(colaPrioritaria))) {
        PCB* procesoAEnviar;
        colaProveniente cola = seleccionarSiguiente(&procesoAEnviar);
        enviarProcesoACPU_RR(procesoAEnviar);
        pidProcesoEnEjecucion = procesoAEnviar->PID;
        ultimoPrioritario = (cola == COLA_PRIORITARIA) ? 1 : 0;
        cpuLibre = 0;
        tiempoTranscurrido = temporal_create();
    }
}

void setVRR() {
    IOGenerica = enviarAIOGenericaVRR;
    IOFinalizada = operacionFinalizadaVRR;
    instWait = waitRR;
    instSignal = signalRR;
    instExit = exitRR;
    interrupcion = interrupcionRR;
    criterioEnvio = criterioEnvioVRR;
}