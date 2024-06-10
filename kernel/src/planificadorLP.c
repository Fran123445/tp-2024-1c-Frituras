#include <planificadorLP.h>

void logProcesosEnCola(char* nombreCola, t_queue* cola) {
    // rompe la abstraccion pero bue, a esta altura es lo que hay

    char* string = string_new();

    void _agregarPIDALista(PCB* proceso) {
        string_append_with_format(&string, "%d, ", proceso->PID);
    };


    list_iterate(cola->elements, (void *) _agregarPIDALista);
    string[strlen(string)-2] = '\0';

    log_info(logger, "[%s]", string);

    free(string);
}

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

void cambiarEstado(PCB* proceso, estado_proceso estado) {
    estado_proceso estadoAnterior = proceso->estado;
    proceso->estado = estado;
    pthread_mutex_lock(&mutexLogger);
    log_info(logger, "PID: %d - Estado Anterior: %s - Estado Actual: %s", proceso->PID, enumEstadoAString(estadoAnterior), enumEstadoAString(proceso->estado));
    pthread_mutex_unlock(&mutexLogger);
}

void enviarAExit(PCB* pcb, motivo_exit motivo) {
    procesoEnExit* aExit = malloc(sizeof(procesoEnExit));
    aExit->pcb = pcb;
    aExit->motivo = motivo;

    pthread_mutex_lock(&mutexExit);
    cambiarEstado(pcb, ESTADO_EXIT);
    queue_push(colaExit, aExit);
    pthread_mutex_unlock(&mutexExit);
    
    sem_post(&procesosEnExit);
}

void enviarAReady(PCB* pcb) {
    pthread_mutex_lock(&mutexReady);

    cambiarEstado(pcb, ESTADO_READY);
    queue_push(colaReady, pcb);

    // No se si dejar esto adentro del mutex de ready
    pthread_mutex_lock(&mutexLogger);
    logProcesosEnCola("READY", colaReady);
    pthread_mutex_unlock(&mutexLogger);

    pthread_mutex_unlock(&mutexReady);
}

void vaciarExit() {
    while(1) {
        sem_wait(&procesosEnExit);

        char* motivo;

        pthread_mutex_lock(&mutexExit);
        procesoEnExit* procesoAFinalizar = queue_pop(colaExit);
        pthread_mutex_unlock(&mutexExit);

        pthread_mutex_lock(&mutexListaProcesos);
        list_remove_element(listadoProcesos, procesoAFinalizar->pcb);
        pthread_mutex_unlock(&mutexListaProcesos);

        t_paquete* paquete = crear_paquete(FIN_PROCESO);
        agregar_int_a_paquete(paquete, procesoAFinalizar->pcb->PID);
        enviar_paquete(paquete, socketMemoria);
        eliminar_paquete(paquete);

        switch(procesoAFinalizar->motivo) {
            case SUCCESS:
                motivo = "SUCCESS"; break;
            case INVALID_RESOURCE:
                motivo = "INVALID RESOURCE"; break;
            case INVALID_WRITE:
                motivo = "INVALID WRITE"; break;
        }

        pthread_mutex_lock(&mutexLogger);
        log_info(logger, "Finaliza el proceso %d - Motivo: %s", procesoAFinalizar->pcb->PID, motivo);
        pthread_mutex_unlock(&mutexLogger);

        sem_post(&gradoMultiprogramacion);
        free(procesoAFinalizar->pcb);
        free(procesoAFinalizar);
    }
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