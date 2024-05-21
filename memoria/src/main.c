#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <utils/server.h>
#include <utils/client.h>
#include <utils/estructurasConexion.h>


int main(int argc, char* argv[]) {
    t_config* config = config_create("memoria.config");
    if (config == NULL) {
        exit(1);
    }; 

// Conexion Memoria como Servidor, IO como cliente
    t_log* logServidorMemoriaIO = log_create("servidor_memoria_io.log","Memoria",false,LOG_LEVEL_TRACE);
    if(logServidorMemoriaIO == NULL){
        fprintf(stderr, "Error al crear log");
        return 1;
    }
    pthread_t esperarClienteIO;

    int servidorParaIO = iniciar_servidor(config_get_string_value(config, "PUERTO_ESCUCHA"),logServidorMemoriaIO);

    t_conexion_escucha* servidorMemoriaParaIO = malloc(sizeof(t_conexion_escucha));
    servidorMemoriaParaIO->modulo = MEMORIA;
    servidorMemoriaParaIO->socket_servidor= servidorParaIO;

    pthread_create(&esperarClienteIO,NULL,(void*) esperarClienteIO, servidorMemoriaParaIO);
    pthread_detach(esperarClienteIO);


// Conexion Memoria como servidor, CPU como cliente
    t_log* logServidorMemoriaCPU = log_create("servidor_memoria_cpu.log","Memoria",false,LOG_LEVEL_TRACE);
    if(logServidorMemoriaCPU == NULL){
        fprintf(stderr, "Error al crear log");
        return 1;
    }

    pthread_t esperarClienteCPU;

    int servidorParaCPU = iniciar_servidor(config_get_string_value(config, "PUERTO_ESCUCHA"),logServidorMemoriaCPU);

    t_conexion_escucha* servidorMemoriaCPU = malloc(sizeof(t_conexion_escucha));
    servidorMemoriaCPU->modulo= MEMORIA;
    servidorMemoriaCPU->socket_servidor= servidorParaCPU;

    pthread_create(&esperarClienteCPU, NULL, (void*) esperarClienteCPU, servidorMemoriaCPU);
    pthread_detach(esperarClienteCPU);

// Conexion Memoria como Servidor y Kernel como cliente
    t_log* logServidorMemoriaKernel = log_create("servidor_memoria_kernel.log","Memoria",true,LOG_LEVEL_TRACE);
    if(logServidorMemoriaKernel == NULL){
        fprintf (stderr, "Error al crear log");
        return 1;
    }

    pthread_t esperarClienteKernel;

    int servidorParaKernel = iniciar_servidor(config_get_string_value(config, "PUERTO_ESCUCHA"), logServidorMemoriaKernel);

    t_conexion_escucha* servidorMemoriaKernel = malloc (sizeof(t_conexion_escucha));
    servidorMemoriaKernel->modulo = MEMORIA;
    servidorMemoriaKernel->socket_servidor= servidorParaKernel;

    pthread_create(&esperarClienteKernel, NULL, (void*) esperarClienteKernel, servidorMemoriaKernel);
    pthread_detach (esperarClienteKernel);

    // char* ipCPU = config_get_string_value(config,"IP_CPU");
   // int socketCPUDispatch = crear_conexion(ipCPU, config_get_string_value(config, "PUERTO_CPU_DISPATCH"),MEMORIA);

    free(servidorMemoriaParaIO);
    free(servidorMemoriaCPU);
    free(servidorMemoriaKernel);
    log_destroy(logServidorMemoriaKernel);
    log_destroy(logServidorMemoriaIO);
    log_destroy(logServidorMemoriaCPU);
    config_destroy(config);

    return 0;
}



