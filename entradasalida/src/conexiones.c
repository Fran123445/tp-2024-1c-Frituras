#include "conexiones.h"

int conectarAKernel(void) {
    t_config* config = config_create("entradasalida.config");
    if (config == NULL) {
        exit(1);
    }; 

    char* ip = config_get_string_value(config, "IP_KERNEL");
    char* puerto = config_get_string_value(config, "PUERTO_KERNEL");

    int conexion = crear_conexion(ip, puerto);
    
    return conexion;
}