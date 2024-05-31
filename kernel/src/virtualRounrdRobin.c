#include <virtualRoundRobin.h>

// indica que si el ultimo proceso enviado salio o no de la cola prioritaria
int ultimoPrioritario = 0;

int pidProcesoEnEjecucion;

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