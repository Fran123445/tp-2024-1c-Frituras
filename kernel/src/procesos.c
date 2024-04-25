#include <procesos.h>

t_queue* enumEstadoACola(int estado) {
    switch (estado)
    {
        case NEW:
            return colaNew;
        case BLOCKED:
            return colaBlocked;
        case EXIT:
            return colaExit;
        case READY:
            return colaReady;
        default:
            return NULL;
    }
}

PCB* hallarPCB(int PID) {
    bool _mismoPID(PCB* proceso) {
        return proceso->PID == PID;
    };

    return list_find(listadoProcesos, (void *) _mismoPID);
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

    // poner un semaforo para sincronizar siguientePID

    nuevoPCB->PID = siguientePID;
    nuevoPCB->estado = NEW;
    //nuevoPCB->quantum = quantum;
    //Agregar el de program counter
    //Agregar el de registrosCPU

    // aca tambien va un semaforo
    // tambien "si el grado de multiprogramacion lo permite, va a ready"
    queue_push(colaNew, nuevoPCB);
    list_add(listadoProcesos, nuevoPCB);

    log_info(logger, "Se crea el proceso %d en NEW", siguientePID);

    siguientePID += 1;
}

void finalizarProceso(int PID) {
    PCB* PCB = hallarPCB(PID);

    if (PCB == NULL) {
        log_error(logger, "El proceso %d no existe", PID);
        return;
    }

    t_queue* colaProceso;

    // semaforos semaforos

    list_remove_element(listadoProcesos, PCB);

    // Habria que ver que hacer si el proceso esta en estado EXEC
    colaProceso = enumEstadoACola(PCB->estado);

    sacarProceso(colaProceso, PCB);
    queue_push(colaExit, PCB);
}