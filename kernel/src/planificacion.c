#include <planificacion.h>

sem_t procesosEnExit;
pthread_mutex_t mutexExit;
pthread_mutex_t mutexNew;
pthread_mutex_t mutexListaProcesos;

void inicializarSemaforosYMutex() {
    sem_init(&procesosEnExit, 0, 0);
    pthread_mutex_init(&mutexExit, NULL);
    pthread_mutex_init(&mutexNew, NULL);
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
