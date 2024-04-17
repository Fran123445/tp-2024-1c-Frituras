#include <stdlib.h>
#include <stdio.h>
#include <utils/client.h>
#include <utils/server.h>

//Falta agregar hilos para diferentes conexiones 

int main(int argc, char* argv[]) {
    t_config* nuevo_config = config_create("entradasalida.config");
    if (nuevo_config == NULL) {
        exit(1);
    }; 

    t_conexion* conexion_kernel = malloc(sizeof(t_conexion));

    conexion_kernel->config = nuevo_config;
    conexion_kernel->ip = "IP_KERNEL";
    conexion_kernel->puerto = "PUERTO_KERNEL";
    conexion_kernel->handshake_envio = IO;


    conectarse_a(conexion_kernel);
    
    free(conexion_kernel);


    return 0;
}
