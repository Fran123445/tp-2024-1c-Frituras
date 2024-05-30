#include <procesos.h>


char* enumEstadoAString(estado_proceso estado) {
    char* string;

    // podria haber puesto un return en cada case, pero gcc me tiraba un warning
    // y me molestaba
    switch (estado)
    {
        case ESTADO_NEW: string = "NEW"; break;
        case ESTADO_READY: string = "READY"; break;
        case ESTADO_BLOCKED: string = "BLOCKED"; break;
        case ESTADO_EXEC: string = "EXEC"; break;
        case ESTADO_EXIT: string = "EXIT"; break;
    }

    return string;
}

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

registros_cpu inicializarRegistrosCPU() {
    registros_cpu reg;
    reg.AX = 0;
    reg.BX = 0;
    reg.CX = 0;
    reg.DX = 0;
    reg.EAX = 0;
    reg.EBX = 0;
    reg.ECX = 0;
    reg.EDX = 0;
    reg.SI = 0;
    reg.DI = 0;
    reg.PC = 0;

    return reg;
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
    nuevoPCB->quantum = quantumInicial;
    nuevoPCB->programCounter = 0;
    nuevoPCB->registros = inicializarRegistrosCPU();
    nuevoPCB->recursosAsignados = string_array_new();

    pthread_mutex_lock(&mutexListaProcesos);
    pthread_mutex_lock(&mutexNew);

    queue_push(colaNew, nuevoPCB);
    list_add(listadoProcesos, nuevoPCB);

    pthread_mutex_unlock(&mutexNew);    
    pthread_mutex_unlock(&mutexListaProcesos);

    t_paquete* paquete = crear_paquete(CREACION_PROCESO);
    agregar_a_paquete(paquete, &siguientePID, sizeof(int));
    agregar_a_paquete(paquete, path, strlen(path)+1);
    enviar_paquete(paquete, socketMemoria);

    eliminar_paquete(paquete);

    pthread_mutex_lock(&mutexLogger);
    log_info(logger, "Se crea el proceso %d en NEW", siguientePID);
    pthread_mutex_unlock(&mutexLogger);

    recibir_operacion(socketMemoria);

    siguientePID += 1;

    sem_post(&procesosEnNew);
}

void finalizarProceso(int PID) {
    /*  eventualmente se rehará */
}