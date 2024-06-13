#include <stdlib.h>
#include <stdio.h>
#include <utils/client.h>
#include <utils/server.h>
#include <utils/serializacion.h>

void iniciarInterfazGenerica(int socket, t_config* config, char* nombre){

    int tiempo_pausa = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");

    t_paquete* paquete = crear_paquete(CONEXION_IOGENERICA);
    agregar_string_a_paquete(paquete, nombre);
    enviar_paquete(paquete ,socket);
    eliminar_paquete(paquete);

    while (1) {
       ssize_t reciv = recibir_operacion(socket);

        if (reciv < 0) {
            exit(-1);
        }
        
        t_buffer* buffer = recibir_buffer(socket);
        int unidades_trabajo = buffer_read_int(buffer);
        sleep(tiempo_pausa * unidades_trabajo);

        t_paquete* paquete = crear_paquete(OPERACION_FINALIZADA);
        enviar_paquete(paquete ,socket);
        eliminar_paquete(paquete);

    }

}


int main(int argc, char* argv[]) {

    t_config* nuevo_config = config_create("entradasalida.config");
    if (nuevo_config == NULL) {
        exit(1);
    }; 

    int conexion_kernel = crear_conexion(config_get_string_value(nuevo_config,"IP_KERNEL"), config_get_string_value(nuevo_config, "PUERTO_KERNEL"), IO);
    int conexion_memoria = crear_conexion(config_get_string_value(nuevo_config,"IP_MEMORIA"), config_get_string_value(nuevo_config, "PUERTO_MEMORIA"), IO);
    char* tipo = config_get_string_value(nuevo_config,"TIPO_INTERFAZ");


    t_paquete* paquete1 = crear_paquete(CONEXION_STDIN);
    enviar_paquete(paquete1, conexion_memoria);
    t_paquete* paquete = crear_paquete(ACCESO_ESPACIO_USUARIO_ESCRITURA);
    char* prueba = "Hola";
    uint32_t df = 10;
    agregar_uint32_a_paquete(paquete, df);
    agregar_string_a_paquete(paquete, prueba);
    enviar_paquete(paquete, conexion_memoria);
    eliminar_paquete(paquete);

    return 0;
}