#include <planificacion.h>

sem_t procesosEnNew;
sem_t procesosEnExit;
sem_t gradoMultiprogramacion;
sem_t cpuDisponible;
pthread_mutex_t mutexExit;
pthread_mutex_t mutexNew;
pthread_mutex_t mutexReady;
pthread_mutex_t mutexBlocked;
pthread_mutex_t mutexListaProcesos;

void procesosReadyLog(char** lista) {
    void _agregarPIDALista(PCB* proceso) {
        if(proceso->estado == READY) {
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
        pthread_mutex_lock(&mutexListaProcesos);

        PCB* proceso = queue_pop(colaExit);
        list_remove_element(listadoProcesos, proceso);
        free(proceso);

        pthread_mutex_unlock(&mutexListaProcesos);
        pthread_mutex_unlock(&mutexExit);
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
        proceso->estado = READY;

        /* no debe ser ni la forma mas eficiente ni la mas elegante
        para hacer este log, pero funciona */
        procesosReadyLog(&listaReady);
        listaReady[strlen(listaReady)-2] = '\0';

        log_info(logger, "Cola READY: [%s]", listaReady);
        
        pthread_mutex_unlock(&mutexReady);
        pthread_mutex_unlock(&mutexNew);
    }
}

void ejecutarSiguiente() {

    while(1) {
        sem_wait(&cpuDisponible);
        pthread_mutex_lock(&mutexReady);
        PCB* proceso = queue_pop(colaReady);
        proceso->estado = EXEC;
        pthread_mutex_unlock(&mutexReady);
    
        enviarProcesoACPU(proceso);
    }
}

void enviarProcesoACPU(PCB* proceso) {
    //serializar
    //send
}

void recibirDeCPU() {
    while(1) {
        //recv
        //deserializar
        //planificar
    }
}

void planificarRecibidoPorFIFO(t_dispatch* procesoRecibido) {
    PCB* proceso = procesoRecibido->proceso;
    switch (procesoRecibido->motivo) {
        case INST_WAIT:
            // todavia no me fije que hace wait
            break;
        case INST_SIGNAL:
            // todavia no me fije que hace signal
            break;
        case SOLICITUD_IO:
            // enviarABlocked(proceso);
            break;
        default:
            // enviarAExit(proceso);
            break;
    }
}