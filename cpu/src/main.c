#include <stdlib.h>
#include <stdio.h>
#include <utils/client.h>
#include <utils/server.h>
#include <conexiones.h>

int main(int argc, char* argv[]) {
    t_config* nuevo_config = config_create("cpu.config");
    if (nuevo_config == NULL) {
    exit(1);
    }; 

    t_conexion* memoria = {
        nuevo_config,
        "IP_MEMORIA",
        "PUERTO_MEMORIA"
    };

    /*Escuchar Conexiones
    pthread_t thread;
    pthread_create(&thread,
						NULL,
						(void*)escucharConexionesCpu,
						NULL);
	pthread_join(thread,NULL);
*/
    //Recibir Conexiones
    
    pthread_t thread2;
    pthread_create(&thread2,
						NULL,
						(void*)conectarse_a,
						memoria);
	pthread_join(thread2,NULL);
    

    return 0;
}
