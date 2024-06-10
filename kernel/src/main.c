#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <commons/config.h>
#include <utils/server.h>
#include <utils/client.h>
#include "consola.h"
#include "planificadorLP.h"
#include "planificacion.h"
#include "interfaces.h"
#include "roundRobin.h"
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

void inicializarColas() {
    colaNew = queue_create();
    colaReady = queue_create();
    colaExit = queue_create();
    interfacesConectadas = list_create();
    listadoProcesos = list_create();
}

void liberarVariablesGlobales() {    
    pthread_join(pth_colaExit, NULL);
    pthread_join(pth_colaNew, NULL);
    pthread_join(pth_colaReady, NULL);
    pthread_join(pth_recibirProc, NULL);
    log_destroy(logger);
    queue_destroy_and_destroy_elements(colaNew, free);
    queue_destroy_and_destroy_elements(colaExit, free);
    queue_destroy_and_destroy_elements(colaReady, free);
    list_destroy(interfacesConectadas); //seguramente tenga que hager un destroy and destroy eleements
    list_destroy(listadoProcesos); // si hay un proceso en exec me queda leakeando memoria, tengo que ver como lo arreglo
    liberar_conexion(socketCPUDispatch);
    liberar_conexion(socketCPUInterrupt);
    liberar_conexion(socketMemoria);
}

void seleccionarAlgoritmoPlanificacion(t_config* config) {
    char* algoritmo = config_get_string_value(config, "ALGORITMO_PLANIFICACION");

    if(!strcmp(algoritmo, "FIFO")) {
        planificar = planificarPorFIFO;
        iniciarFIFO();
    } else if (!strcmp(algoritmo, "RR")) {
        planificar = planificarPorRR;
        iniciarRR();
    } else {
        //VRR
    }
}

int main(int argc, char* argv[]) {
    t_config* config = config_create("kernel.config");
    if (config == NULL) {
        exit(1);
    }; 

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
    inicializarSemaforosYMutex(config_get_int_value(config, "GRADO_MULTIPROGRAMACION"));

    seleccionarAlgoritmoPlanificacion(config);

    leerRecursosDeConfig(config);

    solicitarInput();
    
    config_destroy(config);
    shutdown(servidorIO, SHUT_RD);
    free(servidorKernel);
    liberarVariablesGlobales();
    pthread_join(esperarClientes, NULL);
    log_destroy(logServidor);

    return 0;

}