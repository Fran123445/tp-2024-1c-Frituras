#include <stdlib.h>
#include <stdio.h>
#include <conexiones.h>

int main(int argc, char* argv[]) {
    t_config* nuevo_config = config_create("cpu.config");
    if (nuevo_config == NULL) {
        exit(1);
    }; 

    t_conexion* memoria = malloc(sizeof(t_conexion));

    memoria->config = nuevo_config;
    memoria->ip = "IP_MEMORIA";
    memoria->puerto = "PUERTO_MEMORIA";

    //Escuchar Conexiones
    pthread_t threadEscucha;
    pthread_create(&threadEscucha,
						NULL,
						(void*)escucharConexionesCpu,
						NULL);

    //Conectarse a memoria
    pthread_t threadEnvio;
    pthread_create(&threadEnvio,
						NULL,
						(void*)conectarse_a,
						memoria);
                
	pthread_join(threadEscucha,NULL);
	pthread_join(threadEnvio,NULL);

    free(memoria);

    return 0;
}
