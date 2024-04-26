#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <commons/config.h>
#include <utils/server.h>
#include <utils/client.h>
#include "consola.h"
#include "procesos.h"
#include "planificacion.h"

int siguientePID;
int gradoMultiprogramacion;
t_queue* colaNew;
t_queue* colaReady;
t_queue* colaBlocked;
t_queue* colaExit;

t_list* listadoProcesos;

t_log* logger;

pthread_t pth_colaExit;

void inicializarColas() {
    colaNew = queue_create();
    colaReady = queue_create();
    colaBlocked = queue_create();
    colaExit = queue_create();
    listadoProcesos = list_create();
}

void liberarMemoria() {
    log_destroy(logger);
    queue_destroy_and_destroy_elements(colaNew, free);
    queue_destroy_and_destroy_elements(colaExit, free);
    queue_destroy_and_destroy_elements(colaBlocked, free);
    queue_destroy_and_destroy_elements(colaReady, free);
    list_destroy(listadoProcesos);
}

void crearHilos() {
    pthread_create(&pth_colaExit,
						NULL,
						(void*) vaciarExit,
						NULL);
    pthread_detach(pth_colaExit);
}

int main(int argc, char* argv[]) {
    t_config* nuevo_config = config_create("kernel.config");
    if (nuevo_config == NULL) {
        exit(1);
    }; 

    t_conexion* cpuDispatch = malloc(sizeof(t_conexion));

    cpuDispatch->config = nuevo_config;
    cpuDispatch->ip = "IP_CPU";
    cpuDispatch->puerto = "PUERTO_CPU_DISPATCH";
    cpuDispatch->modulo = KERNEL;

    t_conexion* cpuInterrupt = malloc(sizeof(t_conexion));

    cpuInterrupt->config = nuevo_config;
    cpuInterrupt->ip = "IP_CPU";
    cpuInterrupt->puerto = "PUERTO_CPU_INTERRUPT";
    cpuInterrupt->modulo = KERNEL;

    t_conexion* memoria = malloc(sizeof(t_conexion));

    memoria->config = nuevo_config;
    memoria->ip = "IP_MEMORIA";
    memoria->puerto = "PUERTO_MEMORIA";
    memoria->modulo = KERNEL;
    

    t_conexion_escucha* oyente = malloc(sizeof(t_conexion_escucha));

    oyente->config = nuevo_config;
    oyente->puerto = "PUERTO_ESCUCHA";
    oyente->log = "servidor_kernel.log";
    oyente->nombre_modulo = "kernel";
    oyente->modulo = KERNEL;

    //crear conexiones
    /*conectarse_a(cpuDispatch);
    conectarse_a(cpuInterrupt);
    conectarse_a(memoria);*/

    //crear servidor
    //escucharConexiones(oyente);

    free(cpuDispatch);
    free(cpuInterrupt);
    free(memoria);
    free(oyente);

    //Ese desastre que esta ahi arriba hay que refactorizarlo

    siguientePID = 0;
    gradoMultiprogramacion = config_get_int_value(nuevo_config, "GRADO_MULTIPROGRAMACION");
    logger = log_create("Kernel.log", "Kernel", false, LOG_LEVEL_TRACE);

    inicializarColas();
    inicializarSemaforosYMutex();
    crearHilos();

    solicitarInput();

    config_destroy(nuevo_config);
    liberarMemoria();

    return 0;
}
