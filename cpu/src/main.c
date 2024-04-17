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
    memoria->handshake = CPU;

    t_conexion_escucha* oyente_dispatch = malloc(sizeof(t_conexion_escucha));

    oyente_dispatch->config = nuevo_config;
    oyente_dispatch->puerto = "PUERTO_ESCUCHA_DISPATCH";
    oyente_dispatch->log = "servidorDispatch.log";
    oyente_dispatch->nombre_modulo = "cpuDispatch";
    oyente_dispatch->handshake = CPU_DISPATCH;

    t_conexion_escucha* oyente_interrupt = malloc(sizeof(t_conexion_escucha));

    oyente_interrupt->config = nuevo_config;
    oyente_interrupt->puerto = "PUERTO_ESCUCHA_INTERRUPT";
    oyente_interrupt->log = "servidorInterrupt.log";
    oyente_interrupt->nombre_modulo = "cpuInterrupt";
    oyente_interrupt->handshake = CPU_INTERRUPT;


    //Conectarse a memoria
    pthread_t threadEnvio;
    pthread_create(&threadEnvio,
						NULL,
						(void*)conectarse_a,
						memoria);

    //Escuchar Conexiones
    pthread_t threadEscucha;
    pthread_create(&threadEscucha,
						NULL,
						(void*)escucharConexiones,
						NULL);

    pthread_t threadEscucha2;
    pthread_create(&threadEscucha2,
						NULL,
						(void*)escucharConexiones,
						NULL);
                
	pthread_join(threadEscucha,NULL);
    pthread_join(threadEscucha2,NULL);
	pthread_join(threadEnvio,NULL);

    free(memoria);

    return 0;
}
