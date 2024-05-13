#include <procesos.h>

t_queue* enumEstadoACola(int estado) {
    switch (estado)
    {
        case ESTADO_NEW:
            return colaNew;
        //case ESTADO_BLOCKED:
        //    return colaBlocked; Tengo que ver que hacer con esto
        case ESTADO_EXIT:
            return colaExit;
        case ESTADO_READY:
            return colaReady;
        default:
            return NULL;
    }
}

PCB* hallarPCB(int PID) {
    bool _mismoPID(PCB* proceso) {
        return proceso->PID == PID;
    };

    pthread_mutex_lock(&mutexListaProcesos);
    PCB* encontrado = list_find(listadoProcesos, (void *) _mismoPID);
    pthread_mutex_unlock(&mutexListaProcesos);

    return encontrado;
}

void sacarProceso(t_queue* cola, PCB* proceso) {
    int i;
    t_queue* colaTemporal = queue_create();
    PCB* aux;
    int cantidadElementos = queue_size(cola);

    for(i = 0; i < cantidadElementos; i++) {
        aux = queue_pop(cola);
        if (aux->PID != proceso->PID) {
            queue_push(colaTemporal, aux);
        }
    }

    free(cola->elements); // esto esta porque habia un leak, no se si es la mejor solucion
    *cola = *colaTemporal;
    free(colaTemporal);
}

void iniciarProceso(char* path) {
    PCB* nuevoPCB = malloc(sizeof(PCB));

    nuevoPCB->PID = siguientePID;
    nuevoPCB->estado = ESTADO_NEW;
    //nuevoPCB->quantum = quantum;
    //Agregar el de program counter
    //Agregar el de registrosCPU

    pthread_mutex_lock(&mutexNew);
    queue_push(colaNew, nuevoPCB);
    pthread_mutex_unlock(&mutexNew);
    
    pthread_mutex_lock(&mutexListaProcesos);
    list_add(listadoProcesos, nuevoPCB);
    pthread_mutex_unlock(&mutexListaProcesos);

    log_info(logger, "Se crea el proceso %d en NEW", siguientePID);

    siguientePID += 1;

    sem_post(&procesosEnNew);
}

void finalizarProceso(int PID) {
    /*  eventualmente se rehará */
}