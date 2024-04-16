#include <stdlib.h>
#include <stdio.h>
#include <utils/client.h>
#include <utils/server.h>

int main(int argc, char* argv[]) {
    t_config* nuevo_config = config_create("entradasalida.config");
    if (nuevo_config == NULL) {
        exit(1);
    }; 

    t_conexion* kernel = malloc(sizeof(t_conexion));

    kernel->config = nuevo_config;
    kernel->ip = "IP_KERNEL";
    kernel->puerto = "PUERTO_KERNEL";

    conectarse_a(kernel);
    
    free(kernel);


    return 0;
}
