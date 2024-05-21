#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <utils/server.h>
#include <utils/client.h>
#include "instrucciones.h"

int socket_memoria;

int main(int argc, char* argv[]) {
    
    t_config* config = config_create("cpu.config");
    if (config == NULL) {
        exit(1);
    };

    t_log* log_serv_dispatch = log_create("servidorDispatch", "CPU", true, LOG_LEVEL_TRACE);
    t_log* log_serv_interrupt = log_create("servidorInterrupt", "CPU", true, LOG_LEVEL_TRACE);

    int servidor_dispatch = iniciar_servidor(config_get_string_value(config,"PUERTO_ESCUCHA_DISPATCH"),log_serv_dispatch);

    t_conexion_escucha* oyente_dispatch = malloc(sizeof(t_conexion_escucha));
    oyente_dispatch->socket_servidor = servidor_dispatch;
    oyente_dispatch->modulo = CPU;

    int servidor_interrupt = iniciar_servidor(config_get_string_value(config,"PUERTO_ESCUCHA_INTERRUPT"),log_serv_interrupt);

    t_conexion_escucha* oyente_interrupt = malloc(sizeof(t_conexion_escucha));
    oyente_interrupt->socket_servidor = servidor_interrupt;
    oyente_interrupt->modulo = CPU;


    socket_memoria = crear_conexion(config_get_string_value(config, "IP_MEMORIA"), config_get_string_value(config, "PUERTO_MEMORIA"), CPU);;

    //Escuchar Conexiones
    pthread_t threadEscuchaDispatch;
    pthread_create(&threadEscuchaDispatch,
						NULL,
						(void*)iniciar_servidor,
						oyente_dispatch);
    
    
    pthread_detach(threadEscuchaDispatch);

    pthread_t threadEscuchaInterrupt;
    pthread_create(&threadEscuchaInterrupt,
						NULL,
						(void*)iniciar_servidor,
						oyente_interrupt);
                
    pthread_detach(threadEscuchaInterrupt);
    
    while(1);



    config_destroy(config);
    free(socket_memoria);
    free(oyente_dispatch);
    free(oyente_interrupt);

    return 0;
}
