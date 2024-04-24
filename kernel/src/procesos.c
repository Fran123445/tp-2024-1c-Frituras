#include <procesos.h>

t_list* enumEstadoACola(int estado) {
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
    list_add(colaNew, nuevoPCB);
    list_add(listadoProcesos, nuevoPCB);

    log_trace(logger, "Se crea el proceso %d en NEW", siguientePID);

    siguientePID += 1;
}

void finalizarProceso(int PID) {
    PCB* PCB = hallarPCB(PID);

    if (PCB == NULL) {
        log_error(logger, "El proceso %d no existe", PID);
        return;
    }

    t_list* colaProceso;

    // semaforos semaforos

    list_remove_element(listadoProcesos, PCB);

    // Habria que ver que hacer si el proceso esta en estado EXEC
    colaProceso = enumEstadoACola(PCB->estado);

    list_remove_element(colaProceso, PCB);

    log_trace(logger, "Finaliza el proceso %d - Motivo: Finalizado por el usuario", PCB->PID);

    free(PCB);
}