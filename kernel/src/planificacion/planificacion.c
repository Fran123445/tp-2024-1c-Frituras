#include <planificacion/planificacion.h>

sem_t procesosEnNew;
sem_t procesosEnExit;
sem_t gradoMultiprogramacion;
sem_t finalizarQuantum;
sem_t quantumFinalizado;
pthread_mutex_t mutexExit;
pthread_mutex_t mutexNew;
pthread_mutex_t mutexReady;
pthread_mutex_t mutexListaProcesos;
pthread_mutex_t mutexListaInterfaces;
int cpuLibre = 1;

void (*IOGenerica)(PCB*, op_code, t_buffer*);
void (*IOFinalizada)(PCB*);
void (*instWait)(PCB*, t_buffer*);
void (*instSignal)(PCB*, t_buffer*);
void (*instExit)(PCB*);
void (*criterioEnvio)();
void (*interrupcion)();

void inicializarSemaforosYMutex(int multiprogramacion) {
    sem_init(&procesosEnNew, 0, 0);
    sem_init(&procesosEnExit, 0, 0);
    sem_init(&gradoMultiprogramacion, 0, multiprogramacion);
    sem_init(&finalizarQuantum, 0, 0);
    sem_init(&quantumFinalizado, 0, 0);
    pthread_mutex_init(&mutexPlanificador, NULL);
    pthread_mutex_init(&mutexExit, NULL);
    pthread_mutex_init(&mutexNew, NULL);
    pthread_mutex_init(&mutexReady, NULL);
    pthread_mutex_init(&mutexListaProcesos, NULL);
    pthread_mutex_init(&mutexListaInterfaces, NULL);
}

void procesoNewAReady() {
    while(1) {
        sem_wait(&gradoMultiprogramacion);
        sem_wait(&procesosEnNew);

        pthread_mutex_lock(&mutexPlanificador);
        planificar(CREACION_PROCESO, NULL, NULL);
        pthread_mutex_unlock(&mutexPlanificador);
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

void actualizarProcesoRecibido(PCB* pcbRecibido, PCB* pcbEnKernel) {
    // esto es porque si no al hacer el reemplazo queda el string_array original
    // leakeando memoria
    string_array_destroy(pcbEnKernel->recursosAsignados);

    *pcbEnKernel = *pcbRecibido;
    
    free(pcbRecibido);
}

void leerBufferYPlanificar(op_code operacion) {
    t_buffer* buffer = recibir_buffer(socketCPUDispatch);

    PCB* procesoExec = buffer_read_pcb(buffer);
    PCB* proceso = hallarPCB(procesoExec->PID);
    actualizarProcesoRecibido(procesoExec, proceso); 
    
    planificar(operacion, proceso, buffer);
    liberar_buffer(buffer);
}

void recibirDeCPU() {
    while(1) {
        op_code operacion = recibir_operacion(socketCPUDispatch);
        pthread_mutex_lock(&mutexPlanificador); // No se si seria el lugar correcto para poner esto, pensando de cara a detener la planificacion
        leerBufferYPlanificar(operacion);
        pthread_mutex_unlock(&mutexPlanificador);
    }
}

void enviarAIOGenerica(PCB* proceso, op_code operacion, t_buffer* buffer) {
    t_interfaz_generica* infoInterfaz = buffer_read_interfaz_generica(buffer);
    t_IOConectada* interfaz = hallarInterfazConectada(infoInterfaz->nombre);

    if (comprobarOperacionValida(interfaz, operacion)) {
        t_solicitudIOGenerica* solicitud = malloc(sizeof(t_solicitudIOGenerica));
        solicitud->proceso = proceso;
        solicitud->unidadesTrabajo = infoInterfaz->unidades_trabajo;

        pthread_mutex_lock(&interfaz->mutex);

        queue_push(interfaz->procesosBloqueados, solicitud);
        cambiarEstado(proceso, ESTADO_BLOCKED);

        char* str = string_new();
        string_append_with_format(&str, "BLOCKED %s", interfaz->nombreInterfaz);
        logProcesosEnCola(ESTADO_BLOCKED, str, interfaz->procesosBloqueados);
        free(str);

        pthread_mutex_unlock(&interfaz->mutex);

        sem_post(&interfaz->semaforo);
    } else {
        enviarAExit(proceso, INVALID_INTERFACE);
    }

    free(infoInterfaz->nombre);
    free(infoInterfaz);
}

int instruccionWait(PCB* proceso, t_buffer* buffer) {
    char* nombreRecurso = buffer_read_string(buffer);
    t_recurso* recurso = hallarRecurso(nombreRecurso);
    free(nombreRecurso);

    if (!recurso) {
        enviarAExit(proceso, INVALID_RESOURCE);
        return 1;
    }

    int recursoTomado = waitRecurso(recurso, proceso);

    if (recursoTomado) { 
        enviarProcesoACPU(proceso);
        return 0;
    }

    return 1;
}

int instruccionSignal(PCB* proceso, t_buffer* buffer) {
    char* nombreRecurso = buffer_read_string(buffer);
    t_recurso* recurso = hallarRecurso(nombreRecurso);
    free(nombreRecurso);

    if (!recurso) {
        enviarAExit(proceso, INVALID_RESOURCE);
        return 1;
    }

    signalRecurso(recurso);

    enviarProcesoACPU(proceso);
    return 0;
}

void planificar(op_code operacion, PCB* proceso, t_buffer* buffer) {
    switch (operacion) {
        case CREACION_PROCESO: // esta operacion viene desde el mismo kernel cuando se hace le paso de NEW a READY
            pthread_mutex_lock(&mutexNew);
            PCB* nuevoProceso = queue_pop(colaNew);
            enviarAReady(nuevoProceso);
            pthread_mutex_unlock(&mutexNew);
            break;
        case ENVIAR_IO_GEN_SLEEP:
            IOGenerica(proceso, operacion, buffer);
            break;
        case OPERACION_FINALIZADA:
            IOFinalizada(proceso);
            break;
        case INSTRUCCION_WAIT:
            instWait(proceso, buffer);
            break;
        case INSTRUCCION_SIGNAL:
            instSignal(proceso, buffer);
            break;
        case INSTRUCCION_EXIT:
            instExit(proceso);
            break;
        case FIN_DE_Q:
            interrupcion(proceso);
            break;
        default:
            log_error(logger, "Instruccion no válida");
            break;
    }

    criterioEnvio();
}

void iniciarPlanificacion() {
    pthread_create(&pth_colaExit,
						NULL,
						(void*) vaciarExit,
						NULL);
    pthread_create(&pth_colaNew,
						NULL,
						(void*) procesoNewAReady,
						NULL);        
    pthread_create(&pth_recibirProc,
						NULL,
						(void*) recibirDeCPU,
						NULL);
}