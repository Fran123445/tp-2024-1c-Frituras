#include "accesoEspacioUsuario.h"

pthread_mutex_t mutex_memoria_contigua = PTHREAD_MUTEX_INITIALIZER;

void imprimir_memoria(void *memoria, size_t direccion_fisica, size_t tamanio_a_escribir) {
    unsigned char *ptr = (unsigned char *)memoria + direccion_fisica;
    for (size_t i = 0; i < tamanio_a_escribir; ++i) {
        printf("%02X ", ptr[i]);
    }
    printf("\n");
}
void escribir_memoria(int socket, int tiempo_retardo, t_config* config){
    op_code cod_op = recibir_operacion(socket);
    if(cod_op == ACCESO_ESPACIO_USUARIO_ESCRITURA){
        t_buffer* buffer = recibir_buffer(socket);
        uint32_t direccion_fisica = buffer_read_uint32(buffer);
        uint32_t tamanio_a_escribir = read_buffer_tamanio(buffer);
        int tam_memoria = config_get_int_value(config, "TAM_MEMORIA");
        if(direccion_fisica + tamanio_a_escribir > tam_memoria){
            fprintf(stderr, "Direccion o tamanio a escribir invalido, sobrepasa la memoria");
            exit(EXIT_FAILURE);
        }
        void* valor_a_escribir = malloc(tamanio_a_escribir);
        if(valor_a_escribir == NULL){
            fprintf(stderr, "Valor a escribir en memoria no asignado");
            liberar_buffer(buffer);
            return;
        }
        buffer_read(buffer, valor_a_escribir);
        usleep(tiempo_retardo * 1000);
        pthread_mutex_lock(&mutex_memoria_contigua);
        memcpy(memoria_contigua+direccion_fisica, valor_a_escribir, tamanio_a_escribir);
        pthread_mutex_unlock(&mutex_memoria_contigua);
        t_paquete* paquete = crear_paquete(ESCRITURA_REALIZADA_OK);
        enviar_paquete(paquete, socket);
        imprimir_memoria(memoria_contigua, direccion_fisica, tamanio_a_escribir);

        free(valor_a_escribir);
        liberar_buffer(buffer);
        eliminar_paquete(paquete);
    }
}