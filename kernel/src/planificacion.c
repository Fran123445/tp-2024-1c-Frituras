#include <planificacion.h>

sem_t procesosEnNew;
sem_t procesosEnReady;
sem_t procesosEnExit;
sem_t gradoMultiprogramacion;
sem_t cpuDisponible;
sem_t llegadaProceso;
pthread_mutex_t mutexExit;
pthread_mutex_t mutexNew;
pthread_mutex_t mutexReady;
pthread_mutex_t mutexListaProcesos;
pthread_mutex_t mutexListaInterfaces;
pthread_mutex_t mutexLogger;

int finalizar = 0;

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
    pthread_mutex_init(&mutexLogger, NULL); // no se si dejarlo aca
}

void vaciarExit() {
    while(1) {
        sem_wait(&procesosEnExit);

        if (finalizar) break;

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

void procesoNewAReady() {

    while(1) {
        sem_wait(&gradoMultiprogramacion);
        sem_wait(&procesosEnNew);

        if (finalizar) break;

        PCB* proceso;
        char* listaReady = string_new();

        listaReady[0] = '\0';

        pthread_mutex_lock(&mutexNew);
        pthread_mutex_lock(&mutexReady);

        proceso = queue_pop(colaNew);
        queue_push(colaReady, proceso);
        cambiarEstado(proceso, ESTADO_READY);

        pthread_mutex_unlock(&mutexReady);
        pthread_mutex_unlock(&mutexNew);

        /* no debe ser ni la forma mas eficiente ni la mas elegante
        para hacer este log, pero funciona */
        procesosReadyLog(&listaReady);
        listaReady[strlen(listaReady)-2] = '\0';

        pthread_mutex_lock(&mutexLogger);
        log_info(logger, "Cola READY: [%s]", listaReady);
        pthread_mutex_unlock(&mutexLogger);
        
        free(listaReady);

        sem_post(&procesosEnReady);    
    }
}

void enviarProcesoACPU(PCB* proceso) {
    t_paquete* paquete = crear_paquete(ENVIO_PCB);
    agregar_PCB_a_paquete(paquete, proceso);
    enviar_paquete(paquete, socketCPUDispatch);
    eliminar_paquete(paquete);
}

PCB* sacarSiguienteDeReady() {
    sem_wait(&procesosEnReady);
    if (finalizar) return NULL;
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
        if (finalizar) break;
        enviarProcesoACPU(proceso);
    }
}

void leerBufferYPlanificar(op_code operacion) {
    t_buffer* buffer = recibir_buffer(socketCPUDispatch);
    planificarRecibido(operacion, buffer);
    liberar_buffer(buffer);
}

void recibirDeCPU() {
    while(1) {
        op_code operacion = recibir_operacion(socketCPUDispatch);
        leerBufferYPlanificar(operacion);
    }
}

void actualizarProcesoRecibido(PCB* pcbRecibido, PCB* pcbEnKernel) {
    // esto es porque si no al hacer el reemplazo queda el string_array original
    // leakeando memoria
    string_array_destroy(pcbEnKernel->recursosAsignados);

    *pcbEnKernel = *pcbRecibido;
    
    free(pcbRecibido);
}

void planificarRecibido(op_code operacion, t_buffer* buffer) {
    PCB* procesoExec = buffer_read_pcb(buffer);
    PCB* proceso = hallarPCB(procesoExec->PID);
    actualizarProcesoRecibido(procesoExec, proceso); 
    
    // Seguramente este switch gigante pase a ser varias funciones individuales
    // por un tema obviamente de legibilidad, pero por ahora se queda asi
    switch (operacion) {
        case ENVIAR_IO_GEN_SLEEP:
            t_interfaz_generica* infoInterfaz = buffer_read_interfaz_generica(buffer);
            t_IOConectado* interfaz = hallarInterfazConectada(infoInterfaz->nombre);
            if (comprobarOperacionValida(interfaz, operacion)) {
                t_solicitudIOGenerica* solicitud = malloc(sizeof(t_solicitudIOGenerica));
                solicitud->proceso = proceso;
                solicitud->unidadesTrabajo = infoInterfaz->unidades_trabajo;
                pthread_mutex_lock(&interfaz->mutex);
                queue_push(interfaz->cola, solicitud);
                cambiarEstado(proceso, ESTADO_BLOCKED);
                pthread_mutex_unlock(&interfaz->mutex);
                sem_post(&interfaz->semaforo);
            } else {
                enviarAExit(proceso, INVALID_WRITE); // no se si seria el motivo mas indicado
            }
            break;
        case INSTRUCCION_WAIT:
            char* nombreRecurso = buffer_read_string(buffer);
            t_recurso* recurso = hallarRecurso(nombreRecurso);
            free(nombreRecurso);

            if (!recurso) {
                enviarAExit(proceso, INVALID_RESOURCE);
                break;
            }

            int recursoTomado = waitRecurso(recurso, proceso);

            if (recursoTomado) { 
                enviarProcesoACPU(proceso);
                return;
            }

        case INSTRUCCION_SIGNAL:
            char* nombreRecurso = buffer_read_string(buffer);
            t_recurso* recurso = hallarRecurso(nombreRecurso);
            free(nombreRecurso);

            if (!recurso) {
                enviarAExit(proceso, INVALID_RESOURCE);
                break;
            }

            signalRecurso(recurso);

            enviarProcesoACPU(proceso);
            return;

        case INSTRUCCION_EXIT:
            enviarAExit(proceso, SUCCESS);
            break;
        default:
            pthread_mutex_lock(&mutexLogger);
            log_error(logger, "Instruccion no válida");
            pthread_mutex_unlock(&mutexLogger);
            break;
    }

    sem_post(&cpuDisponible);
}

void planificacionPorFIFO() {
    pthread_create(&pth_colaExit,
						NULL,
						(void*) vaciarExit,
						NULL);

    pthread_create(&pth_colaNew,
						NULL,
						(void*) procesoNewAReady,
						NULL);

    pthread_create(&pth_colaReady,
						NULL,
						(void*) ejecutarSiguiente,
						NULL);              
    pthread_create(&pth_recibirProc,
						NULL,
						(void*) recibirDeCPU,
						NULL);
}

void finalizarHilos() {
    finalizar = 1;
    sem_post(&cpuDisponible);
    sem_post(&gradoMultiprogramacion);
    sem_post(&procesosEnNew);
    sem_post(&procesosEnReady);
    sem_post(&procesosEnExit);
}