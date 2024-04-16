#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <utils/server.h>
#include <utils/client.h>


int main(int argc, char* argv[]) {
    t_config* nuevo_config = config_create("memoria.config");
    if (nuevo_config == NULL) {
        exit(1);
    }; 

    t_conexion_escucha* oyente = malloc(sizeof(t_conexion_escucha));

    oyente->config = nuevo_config;
    oyente->puerto = "PUERTO_ESCUCHA";
    oyente->log = "servidor_memoria.log";
    oyente->nombre_modulo = "memoria";
    oyente->handshake = MEMORIA;

    escucharConexiones(oyente);

    free(oyente);

    return 0;
}
