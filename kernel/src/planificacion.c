#include <planificacion.h>

sem_t procesosEnNew;
sem_t procesosEnExit;
sem_t gradoMultiprogramacion;
sem_t cpuDisponible;
sem_t llegadaProceso;
pthread_mutex_t mutexExit;
pthread_mutex_t mutexNew;
pthread_mutex_t mutexReady;
pthread_mutex_t mutexBlocked;
pthread_mutex_t mutexListaProcesos;

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
    pthread_mutex_init(&mutexBlocked, NULL);
    pthread_mutex_init(&mutexListaProcesos, NULL);
}

void vaciarExit() {
    while(1) {
        sem_wait(&procesosEnExit);

        pthread_mutex_lock(&mutexExit);
        PCB* proceso = queue_pop(colaExit);
        pthread_mutex_unlock(&mutexExit);

        pthread_mutex_lock(&mutexListaProcesos);
        list_remove_element(listadoProcesos, proceso);
        pthread_mutex_unlock(&mutexListaProcesos);

        free(proceso);

        log_info(logger, "FINALIZA EL PROCESO"); //faltan cosas
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

PCB* sacarSiguienteDeReady() {
    pthread_mutex_lock(&mutexReady);
    PCB* proceso = queue_pop(colaReady);
    proceso->estado = ESTADO_EXEC;
    pthread_mutex_unlock(&mutexReady);

    return proceso;
}

void enviarProcesoACPU(PCB* proceso) {
    t_paquete* paquete = crear_paquete();
    paquete->codigo_operacion = ENVIO_PCB;
    agregar_PCB_a_paquete(paquete, proceso);
    enviar_paquete(paquete, socketCPUDispatch);
    eliminar_paquete(paquete);
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
    switch (inst->tipo) {
        case IO_GEN_SLEEP:
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