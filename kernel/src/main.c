#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <commons/config.h>
#include <utils/server.h>
#include <utils/client.h>
#include <planificacion/planificacion.h>
#include <planificacion/FIFO.h>
#include <planificacion/roundRobin.h>
#include <planificacion/virtualRoundRobin.h>
#include <interfaces/interfaces.h>
#include "consola.h"
#include "planificadorLP.h"
#include "recursos.h"

int socketCPUDispatch;
int socketCPUInterrupt;
int socketMemoria;

int siguientePID;
int quantumInicial;

pthread_mutex_t mutexPlanificador;

t_queue* colaNew;
t_queue* colaReady;
t_queue* colaExit;

t_list* listadoProcesos;
t_list* interfacesConectadas;

t_log* logger;

pthread_t pth_colaExit;
pthread_t pth_colaNew;
pthread_t pth_colaReady;
pthread_t pth_recibirProc;

t_config* config;

char* path_scripts;

int gradoMultiprogramacionActual = 0;

void inicializarColas() {
    colaNew = queue_create();
    colaPrioritaria = queue_create();
    colaReady = queue_create();
    colaExit = queue_create();
    interfacesConectadas = list_create();
    listadoProcesos = list_create();
}

void liberarVariablesGlobales() {    
    log_destroy(logger);
    queue_destroy_and_destroy_elements(colaNew, (void*)liberar_pcb);
    queue_destroy_and_destroy_elements(colaExit, free);
    queue_destroy_and_destroy_elements(colaReady, (void*)liberar_pcb);
    list_destroy_and_destroy_elements(interfacesConectadas, free); // las queues no las libera pero bue, esto se llama cuando termina el programa asi que tampoco le voy a dedicar tanto
    list_destroy_and_destroy_elements(listaRecursos, (void*)freeRec);
    list_destroy(listadoProcesos);
    liberar_conexion(socketCPUDispatch);
    liberar_conexion(socketCPUInterrupt);
    liberar_conexion(socketMemoria);
}

void seleccionarAlgoritmoPlanificacion(t_config* config) {
    char* algoritmo = config_get_string_value(config, "ALGORITMO_PLANIFICACION");

    if(!strcmp(algoritmo, "FIFO")) {
        setFIFO();
    } else if (!strcmp(algoritmo, "RR")) {
        setRR();
    } else {
        setVRR();
    }
    iniciarPlanificacion();
}

int main(int argc, char* argv[]) {
    
    t_config* config = config_create(argv[1]);
    if (config == NULL) {
        exit(1);
    }; 

    path_scripts = config_get_string_value(config, "PATH_SCRIPTS");

    t_log* logServidor = log_create("ServidorKernel.log", "Kernel", false, LOG_LEVEL_TRACE);

    pthread_t esperarClientes;

    int servidorIO = iniciar_servidor(config_get_string_value(config, "PUERTO_ESCUCHA"), logServidor);

    t_conexion_escucha* servidorKernel = malloc(sizeof(t_conexion_escucha));
    servidorKernel->modulo = KERNEL;
    servidorKernel->socket_servidor = servidorIO;

    pthread_create(&esperarClientes,
                    NULL,
                    (void*) esperarClientesIO,
                    servidorKernel);

    char* ipCPU = config_get_string_value(config, "IP_CPU");
    socketCPUDispatch = crear_conexion(ipCPU, config_get_string_value(config, "PUERTO_CPU_DISPATCH"), KERNEL);
    socketCPUInterrupt = crear_conexion(ipCPU, config_get_string_value(config, "PUERTO_CPU_INTERRUPT"), KERNEL);
    socketMemoria = crear_conexion(config_get_string_value(config, "IP_MEMORIA"), config_get_string_value(config, "PUERTO_MEMORIA"), KERNEL);
    int a = 1;
    send(socketCPUDispatch, &a, sizeof(int), 0);


    siguientePID = 0;
    quantumInicial = config_get_int_value(config, "QUANTUM");
    logger = log_create("Kernel.log", "Kernel", false, LOG_LEVEL_TRACE);

    inicializarColas();
    gradoMultiprogramacionActual = config_get_int_value(config, "GRADO_MULTIPROGRAMACION");
    inicializarSemaforosYMutex(gradoMultiprogramacionActual);

    seleccionarAlgoritmoPlanificacion(config);

    leerRecursosDeConfig();

    solicitarInput();
    
    config_destroy(config);
    shutdown(servidorIO, SHUT_RD);
    free(servidorKernel);
    liberarVariablesGlobales();
    pthread_join(esperarClientes, NULL);
    log_destroy(logServidor);

    return 0;

}