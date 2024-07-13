#include "main.h"
#include "interfazFs.h"
#include "interfazGen.h"
#include "interfazStd.h"

int conexion_memoria;
int conexion_kernel;

int main(int argc, char* argv[]) {

    t_config* nuevo_config = config_create(argv[2]);
    if (nuevo_config == NULL) {
        exit(1);
    }; 

    conexion_kernel = crear_conexion(config_get_string_value(nuevo_config,"IP_KERNEL"), config_get_string_value(nuevo_config, "PUERTO_KERNEL"), IO);
    conexion_memoria = crear_conexion(config_get_string_value(nuevo_config, "IP_MEMORIA"), config_get_string_value(nuevo_config, "PUERTO_MEMORIA"), IO);

    char* tipo = config_get_string_value(nuevo_config,"TIPO_INTERFAZ");

    if(!strcmp(tipo,"GENERICA")){
        iniciarInterfazGenerica(nuevo_config, argv[1]);
    }

    if(!strcmp(tipo,"STDIN")){
        iniciarInterfazSTDIN(nuevo_config, "TECLADO");
    }

    if (!strcmp(tipo, "STDOUT")) {
        iniciarInterfazSTDOUT(nuevo_config, argv[1]);
    }

    if (!strcmp(tipo, "DIALFS")) {
        iniciarInterfazDialFS(nuevo_config, "FS");
    }

    free(bitmap);
    return 0;
}