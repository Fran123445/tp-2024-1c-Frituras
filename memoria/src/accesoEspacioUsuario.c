#include "accesoEspacioUsuario.h"

pthread_mutex_t mutex_memoria_contigua = PTHREAD_MUTEX_INITIALIZER;

void* escribir_memoria(int socket){
    t_log* log_memoria = log_create("memoria.log", "Memoria",true, LOG_LEVEL_TRACE);
    
        t_buffer* buffer = recibir_buffer(socket);
        uint32_t direccion_fisica = buffer_read_uint32(buffer);
        uint32_t tamanio_a_escribir = buffer_read_uint32(buffer);
        uint32_t pid = buffer_read_int(buffer);

        if(direccion_fisica + tamanio_a_escribir > tam_memoria){
            fprintf(stderr, "Direccion o tamanio a escribir invalido, sobrepasa la memoria");
            exit(EXIT_FAILURE);
        }

        void* valor_a_escribir = malloc(tamanio_a_escribir);
        if(valor_a_escribir == NULL){
            fprintf(stderr, "Valor a escribir en memoria no asignado");
            liberar_buffer(buffer);
            exit(EXIT_FAILURE);
        }
        buffer_read(buffer, valor_a_escribir);

        usleep(tiempo_retardo * 1000);
        log_info(log_memoria,"PID: %d  - Acción: ESCRIBIR, Dirección física: %d - Tamaño: %d", pid, direccion_fisica, tamanio_a_escribir);
        
        pthread_mutex_lock(&mutex_memoria_contigua);
        memcpy(memoria_contigua+direccion_fisica, valor_a_escribir, tamanio_a_escribir);
        pthread_mutex_unlock(&mutex_memoria_contigua);

        t_paquete* paquete = crear_paquete(ESCRITURA_REALIZADA_OK);
        enviar_paquete(paquete, socket);

        free(valor_a_escribir);
        liberar_buffer(buffer);
        eliminar_paquete(paquete);
}

void* leer_memoria(int socket){
    t_log* log_memoria = log_create("memoria.log", "Memoria",true, LOG_LEVEL_TRACE);

        t_buffer* buffer = recibir_buffer(socket);
        uint32_t direccion_fisica = buffer_read_uint32(buffer);
        uint32_t tamanio_a_leer = buffer_read_uint32(buffer);
        uint32_t pid = buffer_read_uint32(buffer);

        if(direccion_fisica + tamanio_a_leer > tam_memoria){
            fprintf(stderr, "Direccion o tamanio a leer invalido, sobrepasa la memoria");
            exit(EXIT_FAILURE);
        }

        usleep(tiempo_retardo * 1000);
        log_info(log_memoria,"PID: %d  - Acción: LEER, Dirección física: %d - Tamaño: %d", pid, direccion_fisica, tamanio_a_leer);

        void* contenido_leido = malloc(tamanio_a_leer);
        pthread_mutex_lock(&mutex_memoria_contigua);
        memcpy(contenido_leido, memoria_contigua+direccion_fisica, tamanio_a_leer);
        pthread_mutex_unlock(&mutex_memoria_contigua);

        t_paquete* paquete = crear_paquete(ACCESO_ESPACIO_USUARIO_LECTURA);
        agregar_a_paquete(paquete, contenido_leido, tamanio_a_leer);
        enviar_paquete(paquete, socket);

        free(contenido_leido);
        liberar_buffer(buffer);
        eliminar_paquete(paquete);
}