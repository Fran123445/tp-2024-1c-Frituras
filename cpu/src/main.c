#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <utils/server.h>
#include <utils/client.h>

int main(int argc, char* argv[]) {
    t_config* nuevo_config = config_create("cpu.config");
    if (nuevo_config == NULL) {
        exit(1);
    }; 

    t_conexion* memoria = malloc(sizeof(t_conexion));

    memoria->config = nuevo_config;
    memoria->ip = "IP_MEMORIA";
    memoria->puerto = "PUERTO_MEMORIA";
    memoria->modulo = CPU;

    t_conexion_escucha* oyente_dispatch = malloc(sizeof(t_conexion_escucha));

    oyente_dispatch->config = nuevo_config;
    oyente_dispatch->puerto = "PUERTO_ESCUCHA_DISPATCH";
    oyente_dispatch->log = "servidorDispatch.log";
    oyente_dispatch->nombre_modulo = "cpuDispatch";
    oyente_dispatch->modulo = CPU;

    t_conexion_escucha* oyente_interrupt = malloc(sizeof(t_conexion_escucha));

    oyente_interrupt->config = nuevo_config;
    oyente_interrupt->puerto = "PUERTO_ESCUCHA_INTERRUPT";
    oyente_interrupt->log = "servidorInterrupt.log";
    oyente_interrupt->nombre_modulo = "cpuInterrupt";
    oyente_interrupt->modulo = CPU;


    conectarse_a(memoria);

    //Escuchar Conexiones
    pthread_t threadEscuchaDispatch;
    pthread_create(&threadEscuchaDispatch,
						NULL,
						(void*)escucharConexiones,
						oyente_dispatch);
    
    

    pthread_t threadEscuchaInterrupt;
    pthread_create(&threadEscuchaInterrupt,
						NULL,
						(void*)escucharConexiones,
						oyente_interrupt);
                
    pthread_join(threadEscuchaInterrupt,NULL);
    pthread_join(threadEscuchaDispatch,NULL);

    free(memoria);
    free(oyente_dispatch);
    free(oyente_interrupt);

    return 0;
}
