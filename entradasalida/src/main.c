#include <stdlib.h>
#include <stdio.h>
#include <utils/client.h>
#include <utils/server.h>

//Falta agregar hilos para diferentes conexiones 
typedef struct {
    char* nombre;
    char* tipo;
    int unidades_trabajo;
} t_interfaz_generica;

void iniciarInterfazIO(t_conexion* conexion, t_interfaz_generica* interfaz, t_config* config, char* nombre ){
    
    //IO
    t_interfaz_generica interfaz;
    int tiempo_pausa = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");

    interfaz.nombre = nombre;
    interfaz.tipo = "TIPO_INTERFAZ";
    //Falta Unidades_TRABAJO

    while (1) {
        t_interfaz_generica* mensaje = esperar_mensaje(conexion);
        if (mensaje == NULL) {
            return -1;
        }
        switch (mensaje) {
            case IO_GEN_SLEEP:
                sleep(tiempo_pausa * interfaz->unidades_trabajo);
                break;
            default:
                break;
        }
        liberar_mensaje(mensaje);
    }

}

int main(int argc, char* argv[]) {
    
    t_config* nuevo_config = config_create("entradasalida.config");
    if (nuevo_config == NULL) {
        exit(1);
    }; 

    t_conexion* conexion_kernel = malloc(sizeof(t_conexion));

    conexion_kernel->config = nuevo_config;
    conexion_kernel->ip = "IP_KERNEL";
    conexion_kernel->puerto = "PUERTO_KERNEL";
    conexion_kernel->modulo = IO;

    //Crear Conexion
    conectarse_a(conexion_kernel);

    // Iniciar la interfaz    
    iniciarInterfazIO(conexion_kernel);// Ver cuando updateamos las conexiones
    
    //Libero
    free(conexion_kernel);

    return 0;
}
