#include "interfazGen.h"

void iniciarInterfazGenerica(t_config* config, char* nombre){

    int tiempo_pausa = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");

    t_paquete* paquete = crear_paquete(CONEXION_IOGENERICA);
    agregar_string_a_paquete(paquete, nombre);
    enviar_paquete(paquete ,conexion_kernel);
    eliminar_paquete(paquete);

    // esto esta unicamente para que no quede memoria colgada
    paquete = crear_paquete(CONEXION_IOGENERICA);
    enviar_paquete(paquete, conexion_memoria);

    while (1) {
       ssize_t reciv = recibir_operacion(conexion_kernel);

        if (reciv <= 0) {
            return;
        }
        
        t_buffer* buffer = recibir_buffer(conexion_kernel);
        int unidades_trabajo = buffer_read_int(buffer);
        usleep(tiempo_pausa * unidades_trabajo);

        t_paquete* paquete = crear_paquete(OPERACION_FINALIZADA);
        enviar_paquete(paquete ,conexion_kernel);
        eliminar_paquete(paquete);
    }
}

