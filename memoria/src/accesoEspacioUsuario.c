#include "accesoEspacioUsuario.h"


void escribir_memoria(int socket, int tiempo_retardo){
    op_code cod_op = recibir_operacion(socket);
    if(cod_op == ACCESO_ESPACIO_USUARIO_ESCRITURA){
        t_buffer* buffer = recibir_buffer(socket);
        uint32_t direccion_fisica = buffer_read_uint32(buffer);
        uint32_t tamanio_a_escribir = buffer_read_uint32(buffer);
        void* valor_a_escribir = malloc(tamanio_a_escribir);
        buffer_read(buffer, valor_a_escribir);
        free(valor_a_escribir);
    }