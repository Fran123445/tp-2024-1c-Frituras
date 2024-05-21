#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <utils/server.h>
#include <utils/client.h>
#include <utils/estructurasConexion.h>


int main(int argc, char* argv[]) {
    t_config* nuevo_config = config_create("memoria.config");
    if (nuevo_config == NULL) {
        exit(1);
    }; 

    t_log* logServidorMemoriaIO = log_create("servidor_memoria_io.log","Memoria",true,LOG_LEVEL_TRACE);
    if(logServidorMemoriaIO == NULL){
        fprintf(stderr, "Error al crear log");
        return 1;
    }
    pthread_t esperarClienteIO;

    int servidorParaIO = iniciar_servidor(config_get_string_value(nuevo_config, "PUERTO_ESCUCHA"),logServidorMemoriaIO);

    t_conexion_escucha* servidorMemoriaParaIO = malloc(sizeof(t_conexion_escucha));
    servidorMemoriaParaIO->modulo = MEMORIA;
    servidorMemoriaParaIO->socket_servidor= servidorParaIO;

    pthread_create(&esperarClienteIO,NULL,(void*) esperarClienteIO, servidorMemoriaParaIO);
    pthread_detach(esperarClienteIO);

    
    t_log* logServidorMemoriaCPU = log_create("servidor_memoria_cpu.log","Memoria",true,LOG_LEVEL_TRACE);
    if(logServidorMemoriaCPU == NULL){
        fprintf(stderr, "Error al crear log");
        return 1;
    }

    pthread_t esperarClienteCPU;

    int servidorParaCPU = iniciar_servidor(config_get_string_value(nuevo_config, "PUERTO_ESCUCHA"),logServidorMemoriaCPU);

    t_conexion_escucha* servidorMemoriaCPU = malloc(sizeof(t_conexion_escucha));
    servidorMemoriaCPU->modulo= MEMORIA;
    servidorMemoriaCPU->socket_servidor= servidorParaCPU;

    pthread_create(&esperarClienteCPU, NULL, (void*) esperarClienteCPU, servidorMemoriaCPU);
    pthread_detach(esperarClienteCPU);

    
    free(servidorMemoriaParaIO);
    free(servidorMemoriaCPU);
    log_destroy(logServidorMemoriaIO);
    log_destroy(logServidorMemoriaCPU);
    config_destroy(nuevo_config);

    return 0;
}



