#include <stdlib.h>
#include <stdio.h>
#include <utils/client.h>
#include <utils/server.h>

//Falta agregar hilos para diferentes conexiones 
typedef struct {
    char* nombre;
    int unidades_trabajo;
} t_interfaz_generica;

void iniciarInterfazGenerica(int socket, t_config* config, char* nombre ){
    
    //IO
    t_interfaz_generica interfaz;

    int tiempo_pausa = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");

    interfaz.nombre = nombre;

    //Falta Unidades_TRABAJO
    send(socket,interfaz.nombre, strlen(interfaz.nombre)+1,0);

    while (1) {
       ssize_t reciv = recv(socket,interfaz.unidades_trabajo,sizeof(int),0);

        if (reciv < 0) {
            return -1;
        }
        sleep(tiempo_pausa * interfaz->unidades_trabajo);
        send(socket, true,sizeof(bool),0);
    }

}

int main(int argc, char* argv[]) {
    
    t_config* nuevo_config = config_create(argv[1]);
    if (nuevo_config == NULL) {
        exit(1);
    }; 

    int conexion_kernel = crear_conexion(config_get_string_value(nuevo_config,"IP_KERNEL"), config_get_string_value(nuevo_config, "PUERTO_KERNEL"), IO);

    char* tipo = config_get_string_value(nuevo_config,"TIPO_INTERFAZ");

    if(!strcmp(*tipo,"IO_GEN_SLEEP")){
        iniciarInterfazGenerica(conexion_kernel);
    }

    //Libero
    free(conexion_kernel);

    return 0;
}
