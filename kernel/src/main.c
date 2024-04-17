#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <utils/server.h>
#include <utils/client.h>

int main(int argc, char* argv[]) {
    t_config* nuevo_config = config_create("kernel.config");
    if (nuevo_config == NULL) {
        exit(1);
    }; 

    t_conexion* cpuDispatch = malloc(sizeof(t_conexion));

    cpuDispatch->config = nuevo_config;
    cpuDispatch->ip = "IP_CPU";
    cpuDispatch->puerto = "PUERTO_ESCUCHA_DISPATCH";
    

    t_conexion_escucha* oyente = malloc(sizeof(t_conexion_escucha));

    oyente->config = nuevo_config;
    oyente->puerto = "PUERTO_ESCUCHA";
    oyente->log = "servidor_kernel.log";
    oyente->nombre_modulo = "kernel";
    oyente->handshake_escucha = KERNEL;

    //conectarse_a(cpuDispatch);
    escucharConexiones(oyente);

    free(cpuDispatch);
    free(oyente);

    return 0;
}
