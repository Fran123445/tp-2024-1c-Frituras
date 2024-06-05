#include <virtualRoundRobin.h>

// indica que si el ultimo proceso enviado salio o no de la cola prioritaria
int ultimoPrioritario = 0;

int pidProcesoEnEjecucion;

pthread_mutex_t mutexColaPrioritaria;
t_queue* colaPrioritaria;
sem_t procesosEnColaPrioritaria;

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
    }
}