#include <stdlib.h>
#include <stdio.h>
#include "conexiones.h"

int main(int argc, char* argv[]) {
    t_config* nuevo_config = config_create("kernel.config");
    if (nuevo_config == NULL) {
        exit(1);
    }; 

    t_conexion* cpuDispatch = malloc(sizeof(t_conexion));

    cpuDispatch->config = nuevo_config;
    cpuDispatch->ip = "IP_CPU";
    cpuDispatch->puerto = "PUERTO_ESCUCHA_DISPATCH";

    conectarse_a(cpuDispatch);

    free(cpuDispatch);

    return 0;
}
