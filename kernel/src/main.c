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
    cpuDispatch->puerto = "PUERTO_CPU_DISPATCH";
    cpuDispatch->modulo = KERNEL;

    t_conexion* cpuInterrupt = malloc(sizeof(t_conexion));

    cpuInterrupt->config = nuevo_config;
    cpuInterrupt->ip = "IP_CPU";
    cpuInterrupt->puerto = "PUERTO_CPU_INTERRUPT";
    cpuInterrupt->modulo = KERNEL;

    t_conexion* memoria = malloc(sizeof(t_conexion));

    memoria->config = nuevo_config;
    memoria->ip = "IP_MEMORIA";
    memoria->puerto = "PUERTO_MEMORIA";
    memoria->modulo = KERNEL;
    

    t_conexion_escucha* oyente = malloc(sizeof(t_conexion_escucha));

    oyente->config = nuevo_config;
    oyente->puerto = "PUERTO_ESCUCHA";
    oyente->log = "servidor_kernel.log";
    oyente->nombre_modulo = "kernel";
    oyente->modulo = KERNEL;

    //crear conexiones
    conectarse_a(cpuDispatch);
    conectarse_a(cpuInterrupt);
    conectarse_a(memoria);
    //crear servidor
    escucharConexiones(oyente);

    free(cpuDispatch);
    free(cpuInterrupt);
    free(oyente);



    return 0;
}
