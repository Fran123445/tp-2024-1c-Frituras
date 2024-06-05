#include <planificacion.h>

sem_t procesosEnNew;
sem_t procesosEnReady;
sem_t procesosEnExit;
sem_t gradoMultiprogramacion;
sem_t cpuDisponible;
sem_t finalizarQuantum;
pthread_mutex_t mutexExit;
pthread_mutex_t mutexNew;
pthread_mutex_t mutexReady;
pthread_mutex_t mutexListaProcesos;
pthread_mutex_t mutexListaInterfaces;
pthread_mutex_t mutexLogger;

void inicializarSemaforosYMutex(int multiprogramacion) {
    sem_init(&procesosEnNew, 0, 0);
    sem_init(&procesosEnExit, 0, 0);
    sem_init(&cpuDisponible, 0, 1); // lo inicializo en 1 porque (entiendo) al arrancar el kernel la cpu no va a estar ocupada con nada
    sem_init(&gradoMultiprogramacion, 0, multiprogramacion);
    sem_init(&finalizarQuantum, 0, 0);
    pthread_mutex_init(&mutexPlanificador, NULL);
    pthread_mutex_init(&mutexExit, NULL);
    pthread_mutex_init(&mutexNew, NULL);
    pthread_mutex_init(&mutexReady, NULL);
    pthread_mutex_init(&mutexListaProcesos, NULL);
    pthread_mutex_init(&mutexListaInterfaces, NULL);
    pthread_mutex_init(&mutexLogger, NULL); // no se si dejarlo aca
}

void procesoNewAReady() {

    while(1) {
        sem_wait(&gradoMultiprogramacion);
        sem_wait(&procesosEnNew);

        pthread_mutex_lock(&mutexNew);

        PCB* proceso = queue_pop(colaNew);
        enviarAReady(proceso);

        pthread_mutex_unlock(&mutexNew);
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

void planificarPorFIFO(op_code operacion, PCB* proceso, t_buffer* buffer) {
    int cpuLibre;
    switch (operacion) {
        case ENVIAR_IO_GEN_SLEEP:
            enviarAIOGenerica(proceso, operacion, buffer);
            break;
        case OPERACION_FINALIZADA:
            enviarAReady(proceso);
            return;
        case INSTRUCCION_WAIT:
            cpuLibre = instruccionWait(proceso, buffer);
            if (cpuLibre) break; else return;
        case INSTRUCCION_SIGNAL:
            cpuLibre = instruccionSignal(proceso, buffer);
            if (cpuLibre) break; else return;
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

void iniciarFIFO() {
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