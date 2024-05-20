#include <planificacion.h>

sem_t procesosEnNew;
sem_t procesosEnExit;
sem_t gradoMultiprogramacion;
sem_t cpuDisponible;
sem_t llegadaProceso;
pthread_mutex_t mutexExit;
pthread_mutex_t mutexNew;
pthread_mutex_t mutexReady;
pthread_mutex_t mutexListaProcesos;
pthread_mutex_t mutexListaInterfaces;

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
}

void procesosReadyLog(char** lista) {
    void _agregarPIDALista(PCB* proceso) {
        if(proceso->estado == ESTADO_READY) {
            string_append_with_format(lista, "%d, ", proceso->PID);
        }
    };
    
    pthread_mutex_lock(&mutexListaProcesos);

    list_iterate(listadoProcesos, (void *) _agregarPIDALista);

    pthread_mutex_unlock(&mutexListaProcesos);
}

void inicializarSemaforosYMutex(int multiprogramacion) {
    sem_init(&procesosEnNew, 0, 0);
    sem_init(&procesosEnExit, 0, 0);
    sem_init(&cpuDisponible, 0, 1); // lo inicializo en 1 porque (entiendo) al arrancar el kernel la cpu no va a estar ocupada con nada
    sem_init(&gradoMultiprogramacion, 0, multiprogramacion);
    sem_init(&llegadaProceso, 0, 0);
    pthread_mutex_init(&mutexExit, NULL);
    pthread_mutex_init(&mutexNew, NULL);
    pthread_mutex_init(&mutexReady, NULL);
    pthread_mutex_init(&mutexListaProcesos, NULL);
    pthread_mutex_init(&mutexListaInterfaces, NULL);
}

void vaciarExit() {

    char* motivo;

    while(1) {
        sem_wait(&procesosEnExit);

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

        log_info(logger, "Finaliza el proceso %d - Motivo: %s", procesoAFinalizar->pcb->PID, motivo);

        free(procesoAFinalizar->pcb);
        free(procesoAFinalizar);
    }
}

void procesoNewAReady() {
    PCB* proceso;
    char* listaReady = string_new();

    while(1) {
        sem_wait(&gradoMultiprogramacion);
        sem_wait(&procesosEnNew);

        listaReady[0] = '\0';

        pthread_mutex_lock(&mutexNew);
        pthread_mutex_lock(&mutexReady);

        proceso = queue_pop(colaNew);
        queue_push(colaReady, proceso);
        proceso->estado = ESTADO_READY;

        pthread_mutex_unlock(&mutexReady);
        pthread_mutex_unlock(&mutexNew);

        /* no debe ser ni la forma mas eficiente ni la mas elegante
        para hacer este log, pero funciona */
        procesosReadyLog(&listaReady);
        listaReady[strlen(listaReady)-2] = '\0';

        log_info(logger, "Cola READY: [%s]", listaReady);
        
    }
}

void enviarProcesoACPU(PCB* proceso) {
    t_paquete* paquete = crear_paquete(ENVIO_PCB);
    agregar_PCB_a_paquete(paquete, proceso);
    enviar_paquete(paquete, socketCPUDispatch);
    eliminar_paquete(paquete);
}

PCB* sacarSiguienteDeReady() {
    pthread_mutex_lock(&mutexReady);
    PCB* proceso = queue_pop(colaReady);
    cambiarEstado(proceso, ESTADO_EXEC);
    pthread_mutex_unlock(&mutexReady);

    return proceso;
}

void ejecutarSiguiente() {
    while(1) {
        sem_wait(&cpuDisponible);
        PCB* proceso = sacarSiguienteDeReady();
        enviarProcesoACPU(proceso);
    }
}

void leerBufferYPlanificar() {
    t_buffer* buffer = recibir_buffer(socketCPUDispatch);
    t_dispatch* proceso = buffer_read_dispatch(buffer);
    planificarRecibido(proceso);
    liberar_buffer(buffer);
}

void recibirDeCPU() {
    while(1) {
        recibir_operacion(socketCPUDispatch);
        leerBufferYPlanificar();
    }
}

void planificarRecibido(t_dispatch* dispatch) {
    PCB* proceso = dispatch->proceso;
    t_instruccion* inst = dispatch->instruccion;
    t_IOConectado* interfaz;
    switch (inst->tipo) {
        case IO_GEN_SLEEP:
            interfaz = hallarInterfazConectada(inst->interfaz);
            if (comprobarOperacionValida(interfaz, inst->tipo)) {
                t_solicitudIOGenerica* solicitud = malloc(sizeof(t_solicitudIOGenerica));
                solicitud->proceso = proceso;
                solicitud->unidadesTrabajo = *(int*) inst->arg1;

                pthread_mutex_lock(&interfaz->mutex);
                queue_push(interfaz->cola, solicitud);
                cambiarEstado(proceso, ESTADO_BLOCKED);
                pthread_mutex_unlock(&interfaz->mutex);

            } else {
                enviarAExit(proceso, INVALID_WRITE); // no se si seria el motivo mas indicado
            }
            break;
        case WAIT:
            // todavia no me fije que hace wait
            break;
        case SIGNAL:
            // todavia no me fije que hace signal
            break;
        case EXIT:
            break;
        default:
            log_error(logger, "Instruccion no válida");
            break;
    }
}

void planificacionPorFIFO() {
    pthread_t pth_colaExit;
    pthread_t pth_colaNew;
    pthread_t pth_colaReady;
    pthread_t pth_recibirProc;

    pthread_create(&pth_colaExit,
						NULL,
						(void*) vaciarExit,
						NULL);
    pthread_detach(pth_colaExit);

    pthread_create(&pth_colaNew,
						NULL,
						(void*) procesoNewAReady,
						NULL);
    pthread_detach(pth_colaNew);

    pthread_create(&pth_colaReady,
						NULL,
						(void*) ejecutarSiguiente,
						NULL);
    pthread_detach(pth_colaReady);

    pthread_create(&pth_recibirProc,
						NULL,
						(void*) recibirDeCPU,
						NULL);
    pthread_detach(pth_recibirProc);
}