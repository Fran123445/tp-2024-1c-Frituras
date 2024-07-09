#include <planificadorLP.h>

void logProcesosEnCola(estado_proceso estado, char* nombreCola, t_queue* cola) {
    // rompe la abstraccion pero bue, a esta altura es lo que hay

    char* string = string_new();

    void (*func)(void*);

    void _agregarPIDALista(PCB* proceso) {
        string_append_with_format(&string, "%d, ", proceso->PID);
    };

    void _agregarPIDBloqueadoALista(void* elemento) {
        PCB* proceso = *(PCB**) elemento;
        string_append_with_format(&string, "%d, ", proceso->PID);
    };

    if (estado == ESTADO_BLOCKED) func = _agregarPIDBloqueadoALista;
    else func = _agregarPIDALista;

    list_iterate(cola->elements, (void *) func);
    string[strlen(string)-2] = '\0';

    log_info(logger, "Cola %s: [%s]", nombreCola, string);

    free(string);
}

void enviarInterrupcion(int PID, op_code motivo) {
    t_paquete* paquete = crear_paquete(motivo);
    agregar_int_a_paquete(paquete, PID);
    enviar_paquete(paquete, socketCPUInterrupt);
    eliminar_paquete(paquete);
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
    log_info(logger, "PID: %d - Estado Anterior: %s - Estado Actual: %s", proceso->PID, enumEstadoAString(estadoAnterior), enumEstadoAString(proceso->estado));
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
    logProcesosEnCola(ESTADO_READY, "READY", colaReady);

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

        liberarRecursos(procesoAFinalizar->pcb);

        t_paquete* paquete = crear_paquete(FIN_PROCESO);
        agregar_int_a_paquete(paquete, procesoAFinalizar->pcb->PID);
        enviar_paquete(paquete, socketMemoria);
        eliminar_paquete(paquete);

        switch(procesoAFinalizar->motivo) {
            case SUCCESS:
                motivo = "SUCCESS"; break;
            case INVALID_RESOURCE:
                motivo = "INVALID RESOURCE"; break;
            case INVALID_INTERFACE:
                motivo = "INVALID INTERFACE"; break;
            case OOM:
                motivo = "OUT OF MEMORY"; break;
            case INTERRUPTED_BY_USER:
                motivo = "INTERRUPTED BY USER"; break;
        }

        log_info(logger, "Finaliza el proceso %d - Motivo: %s", procesoAFinalizar->pcb->PID, motivo);

        sem_post(&gradoMultiprogramacion);
        liberar_pcb(procesoAFinalizar->pcb);
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

    log_info(logger, "Se crea el proceso %d en NEW", siguientePID);

    recibir_operacion(socketMemoria);

    siguientePID += 1;

    logProcesosEnCola(ESTADO_NEW, "NEW", colaNew);

    sem_post(&procesosEnNew);
}

bool sacarDeCola(t_queue* cola, int PID) {
    PCB* proceso = NULL;
    bool _mismoPID(PCB* pcb) {
        return pcb->PID == PID;
    };

    proceso = list_remove_by_condition(cola->elements, (bool (*)(void*)) _mismoPID);

    return proceso != NULL;
}

void sacarProcesoBloqueado(int PID) {
    // quedo bastante fea la cosa pero es lo que hay
    int hallado = 0;

    void _sacarDeColaInterfaz(t_IOConectada* interfaz) {
        if (hallado) return;
        hallado = sacarDeCola(interfaz->procesosBloqueados, PID);
    };

    void _sacarDeColaRecuros(t_recurso* recurso) {
        if (hallado) return;
        hallado = sacarDeCola(recurso->procesosBloqueados, PID);
    };

    list_iterate(interfacesConectadas, (void *) _sacarDeColaInterfaz);
    if (hallado) return;
    list_iterate(listaRecursos, (void *) _sacarDeColaRecuros);
}

void finalizarProceso(int PID) {
    pthread_mutex_lock(&mutexPlanificador);

    PCB* proceso = hallarPCB(PID);
    
    switch(proceso->estado) {
        case ESTADO_EXEC:
            enviarInterrupcion(PID, FINALIZAR_PROCESO);
            pthread_mutex_unlock(&mutexPlanificador);
            return;
        case ESTADO_BLOCKED:
            sacarProcesoBloqueado(PID);
            break;
        case ESTADO_NEW:
            sacarDeCola(colaNew, PID);
            break;
        case ESTADO_READY:
            sacarDeCola(colaReady, PID);
            break;
        default:
            break;
    }

    enviarAExit(proceso, INTERRUPTED_BY_USER);
    pthread_mutex_unlock(&mutexPlanificador);
}