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
t_conexion_escucha* escucha_cpu;
t_conexion_escucha* escucha_kernel;
t_conexion_escucha* escucha_io;
t_config* config;
t_parametros_cpu* params_cpu;
t_parametros_kernel* params_kernel;
t_parametros_io* params_io;
t_bitarray* mapa_de_marcos;

void iniciar_servidores(t_config* config){
    t_log* log_memoria = log_create("memoria.log", "Memoria",true, LOG_LEVEL_TRACE);

    socket_servidor_memoria = iniciar_servidor(config_get_string_value(config, "PUERTO_ESCUCHA"),log_memoria);

    escucha_io= malloc(sizeof(t_conexion_escucha));
    escucha_io->modulo= MEMORIA;
    escucha_io->socket_servidor= socket_servidor_memoria;

  //  socket_kernel = esperar_cliente(socket_servidor_memoria, MEMORIA);
    // socket_cpu = esperar_cliente(socket_servidor_memoria, MEMORIA);


}

void* escuchar_cpu(void* argumento_cpu){
    t_parametros_cpu* params_cpu = (t_parametros_cpu*)argumento_cpu;
    //params_cpu usado para pasar los parámetros al hilo sin problema. ignorar warning.
    int tiempo_retardo = config_get_int_value(config, "RETARDO_RESPUESTA");
    while(1){
        mandar_instruccion_cpu(socket_kernel,socket_cpu, tiempo_retardo);
        resize_proceso(socket_cpu, config,tiempo_retardo);
        acceso_tabla_paginas(socket_cpu,tiempo_retardo);
        escribir_memoria(socket_cpu,tiempo_retardo,config);

    }
}
void* escuchar_kernel(void* argumento_kernel){
    t_parametros_kernel* params_kernel = (t_parametros_kernel*)argumento_kernel;
    //params_kernel usado para pasar los parámetros al hilo sin problema. ignorar warning.
    int tiempo_retardo = config_get_int_value(config, "RETARDO_RESPUESTA");
    while(1){
        abrir_archivo_path(socket_kernel, tiempo_retardo);
        finalizar_proceso(socket_kernel, tiempo_retardo);

    }
}
void* escuchar_io(void* argumento_io){
    t_parametros_io* params_io = (t_parametros_io*)argumento_io;
    //Ignorar warning, params_io se usa para los parámetros del hilo sin problema dentro del main :D
    int tiempo_retardo = config_get_int_value(config, "RETARDO_RESPUESTA");
    esperar_clientes_IO(escucha_io,tiempo_retardo, config);

}

t_bitarray* iniciar_bitmap_marcos(int cant_marcos){
    char* bitarray_memoria_usuario = calloc(cant_marcos/8, sizeof(char));
    if (!bitarray_memoria_usuario){
        fprintf(stderr, "Error al crear puntero al bitarray");
        exit(EXIT_FAILURE);
    }
    t_bitarray* mapa_de_marcos = bitarray_create_with_mode(bitarray_memoria_usuario, cant_marcos, LSB_FIRST); // se lee el bit - significativo primero
    if(mapa_de_marcos == NULL){
        free(bitarray_memoria_usuario);
        fprintf(stderr, "Error al crear el bitarray");
        exit(EXIT_FAILURE);
    }
    return mapa_de_marcos;
}

int main(int argc, char *argv[]){
    lista_de_procesos = list_create();

    config = config_create("memoria.config");
    if (config == NULL){
        exit(1);
    }
    iniciar_servidores(config);
    memoria_contigua = iniciar_memoria(config);
    int cant_marcos = calcular_marcos(config);
    mapa_de_marcos = iniciar_bitmap_marcos(cant_marcos);

    //pthread_t hilo_kernel;
    //pthread_create(&hilo_kernel,NULL, escuchar_kernel, (void*)params_kernel);
   // pthread_t hilo_cpu;
   // pthread_create(&hilo_cpu, NULL, escuchar_cpu, (void*)params_cpu);
    pthread_t hilo_io;
    pthread_create(&hilo_io, NULL,escuchar_io,(void*)params_io);
  //  pthread_join(hilo_cpu, NULL);
 //   pthread_join(hilo_kernel, NULL);
    pthread_join(hilo_io,NULL);
    config_destroy(config);
   // free(escucha_cpu);
   // free(escucha_kernel);
    free(memoria_contigua);
    list_destroy(lista_de_procesos);
    bitarray_destroy(mapa_de_marcos);
    pthread_mutex_destroy(&mutex_bitarray_marcos_libres);
    pthread_mutex_destroy(&mutex_lista_procesos);
    pthread_mutex_destroy(&mutex_log_memoria_io);
    return 0;
}