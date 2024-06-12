#include "accesoEspacioUsuario.h"


void escribir_memoria(int socket, int tiempo_retardo, t_config* config){
    op_code cod_op = recibir_operacion(socket);
    if(cod_op == ACCESO_ESPACIO_USUARIO_ESCRITURA){
        t_buffer* buffer = recibir_buffer(socket);
        uint32_t direccion_fisica = buffer_read_uint32(buffer);
        uint32_t tamanio_a_escribir = buffer_read_uint32(buffer);
        int tam_memoria = config_get_int_value(config, "TAM_MEMORIA");
        if(direccion_fisica + tamanio_a_escribir > tam_memoria){
            fprintf(stderr, "Direccion o tamanio a escribir invalido");
            exit(EXIT_FAILURE);
        }
        void* valor_a_escribir = malloc(tamanio_a_escribir);
        if(valor_a_escribir == NULL){
            fprintf(stderr, "Valor a escribir en memoria no asignado");
            liberar_buffer(buffer);
            return;
        }
        usleep(tiempo_retardo * 1000);
        buffer_read(buffer, valor_a_escribir);
        memcpy(memoria_contigua+direccion_fisica,valor_a_escribir,tamanio_a_escribir);
        t_paquete* paquete = crear_paquete(ESCRITURA_REALIZADA_OK);
        enviar_paquete(paquete, socket);
        free(valor_a_escribir);
        liberar_buffer(buffer);
    }