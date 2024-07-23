#include<stdint.h>
#include "main.h"
#include "memoriaCPU.h"
#include "conexiones.h"
#include "memoriaContigua.h"
#include "accesoEspacioUsuario.h"

int socket_kernel = 0;
int socket_cpu = 0;
int socket_io = 0;
int socket_servidor_memoria;
int tiempo_retardo;
int tam_memoria;
int tam_pagina; 
t_conexion_escucha* escucha_cpu;
t_conexion_escucha* escucha_kernel;
t_conexion_escucha* escucha_io;
t_config* config;
t_bitarray* mapa_de_marcos;
t_log* log_memoria;
char* bitarray_memoria_usuario;
t_log* log_servidor;
void iniciar_servidores(t_config* config){
    log_servidor = log_create("memoriaa.log", "Memoria",true, LOG_LEVEL_TRACE);
    socket_servidor_memoria = iniciar_servidor(config_get_string_value(config, "PUERTO_ESCUCHA"),log_servidor);

    escucha_io= malloc(sizeof(t_conexion_escucha));
    escucha_io->modulo= MEMORIA;
    escucha_io->socket_servidor= socket_servidor_memoria;

    socket_kernel = esperar_cliente(socket_servidor_memoria, MEMORIA);
    socket_cpu = esperar_cliente(socket_servidor_memoria, MEMORIA);

    //log_destroy(log_servidor);
}

void escuchar_cpu(){
    while(1){
    op_code cod_op = recibir_operacion(socket_cpu);
    switch (cod_op) {
        case ENVIO_DE_INSTRUCCIONES:
            mandar_instruccion_cpu(socket_kernel, socket_cpu);
            break;
        case ENVIO_RESIZE:
            resize_proceso(socket_cpu);
            break;
        case ACCESO_TABLAS_PAGINAS:
            acceso_tabla_paginas(socket_cpu);
            break;
        case ACCESO_ESPACIO_USUARIO_LECTURA:
            leer_memoria(socket_cpu);
            break;
        case ACCESO_ESPACIO_USUARIO_ESCRITURA:
            escribir_memoria(socket_cpu);
            break;
        default:
            return;
        }
    }
}

void escuchar_kernel(){
    while(1){
        op_code cod_op = recibir_operacion(socket_kernel);
        switch (cod_op) {
            case CREACION_PROCESO:
                abrir_archivo_path(socket_kernel);
                break;
            case FINALIZAR_PROCESO:
                finalizar_proceso(socket_kernel);
                break;
            default:
                return;
        }
    }
}

void escuchar_io(){
    esperar_clientes_IO(escucha_io);
}

t_bitarray* iniciar_bitmap_marcos(int cant_marcos){
    bitarray_memoria_usuario = calloc(cant_marcos/8, sizeof(char));
    if (!bitarray_memoria_usuario){
        fprintf(stderr, "Error al crear puntero al bitarray");
        exit(EXIT_FAILURE);
    }
    t_bitarray* mapa_de_marcos = bitarray_create_with_mode(bitarray_memoria_usuario, cant_marcos, LSB_FIRST); // se lee el bit - significativo primero
    if(mapa_de_marcos == NULL){
        free(bitarray_memoria_usuario);
        free(mapa_de_marcos);
        fprintf(stderr, "Error al crear el bitarray");
        exit(EXIT_FAILURE);
    }
    return mapa_de_marcos;
}

void enviar_tamanio_pagina_a_cpu(){
    t_paquete* paquete = crear_paquete(ENVIO_TAMANIO_PAGINA);
    agregar_int_a_paquete(paquete, tam_pagina);
    enviar_paquete(paquete, socket_cpu);
    eliminar_paquete(paquete);
}


int main(int argc, char *argv[]){
    config = config_create(argv[1]);
    if (config == NULL){
    fprintf(stderr, "Error en la configuracion");
        exit(1);
    }
    lista_de_procesos = list_create();

    iniciar_servidores(config);
    log_memoria = log_create("memoria.log", "Memoria",true, LOG_LEVEL_TRACE);

    tiempo_retardo = config_get_int_value(config, "RETARDO_RESPUESTA");
    tam_memoria = config_get_int_value(config, "TAM_MEMORIA");
    tam_pagina = config_get_int_value(config, "TAM_PAGINA");

    memoria_contigua = iniciar_memoria(config);
    int cant_marcos = calcular_marcos(config);
    mapa_de_marcos = iniciar_bitmap_marcos(cant_marcos);

    enviar_tamanio_pagina_a_cpu();

    pthread_t hilo_kernel;
    pthread_create(&hilo_kernel,NULL, (void*)escuchar_kernel, NULL);

    pthread_t hilo_cpu;
    pthread_create(&hilo_cpu, NULL, (void*)escuchar_cpu, NULL);

    pthread_t hilo_io;
    pthread_create(&hilo_io, NULL, (void*)escuchar_io, NULL);

    pthread_join(hilo_cpu, NULL);
    pthread_join(hilo_kernel, NULL);

    pthread_detach(hilo_io);

    pthread_mutex_destroy(&mutex_bitarray_marcos_libres);

    free(escucha_cpu);
    free(escucha_kernel);
    free(escucha_io);
    free(memoria_contigua);
    free(bitarray_memoria_usuario);

    bitarray_destroy(mapa_de_marcos);
    list_destroy(lista_de_procesos);
    log_destroy(log_memoria);

    close(socket_servidor_memoria);
    log_destroy(log_servidor);

    config_destroy(config);

    return 0;
}