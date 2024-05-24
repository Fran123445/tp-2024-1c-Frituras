#include<stdint.h>
#include "main.h"
#include "memoriaCPU.h"
#include "conexiones.h"
int socket_kernel;
int socket_cpu;
int socket_io;
int socket_serv_cpu;
int socket_serv_kernel;
int socket_serv_io;
t_conexion_escucha* escucha_cpu;
t_conexion_escucha* escucha_kernel;
t_conexion_escucha* escucha_io;
t_config* config;
t_parametros_cpu* params_cpu;
void iniciar_servidores(t_config* config){
    t_log* log_memoria_kernel = log_create("memoria_kernel", "Memoria",true, LOG_LEVEL_TRACE);
    t_log* log_memoria_cpu = log_create("memoria_cpu.log", "Memoria", true, LOG_LEVEL_TRACE);
    t_log* log_memoria_io = log_create("memoria_io.log", "Memoria", true, LOG_LEVEL_TRACE);

    socket_serv_kernel = iniciar_servidor(config_get_string_value(config, "PUERTO_ESCUCHA"),log_memoria_kernel);
    socket_serv_cpu = iniciar_servidor(config_get_string_value(config,"PUERTO_ESCUCHA"), log_memoria_cpu);
    socket_serv_io = iniciar_servidor(config_get_string_value(config, "PUERTO_ESCUCHA"), log_memoria_io);

    socket_kernel = esperar_cliente(socket_serv_kernel, KERNEL);
    socket_cpu = esperar_cliente(socket_serv_cpu, CPU);
    socket_io = (int)(intptr_t)esperar_clientes_IO(escucha_io);

    escucha_cpu = malloc(sizeof(t_conexion_escucha));
    escucha_cpu->modulo=MEMORIA;
    escucha_cpu->socket_servidor=  socket_cpu;

    escucha_kernel= malloc(sizeof(t_conexion_escucha));
    escucha_kernel->socket_servidor=socket_cpu;
    escucha_kernel->modulo= MEMORIA;

    escucha_io= malloc(sizeof(t_conexion_escucha));
    escucha_io->modulo= MEMORIA;
    escucha_io->socket_servidor= socket_io;

}
void* escuchar_cpu(void* argumento){
    t_parametros_cpu* params_cpu = (t_parametros_cpu*)argumento;
    //params_cpu usado para pasar los parámetros al hilo sin problema. ignorar warning.
    int tiempo_retardo = config_get_int_value(config, "PUERTO_ESCUCHA");
    while(1){
        recibir_proceso_cpu(socket_cpu);
        mandar_instruccion_cpu(socket_kernel,socket_cpu, tiempo_retardo);
    }
}
void* escuchar_kernel(){
    while(1){
        creacion_proceso(socket_kernel);
        abrir_archivo_path(socket_kernel);
    }
}
//void* escuchar_io(){}

int main(int argc, char *argv[]){

    t_config *config = config_create("memoria.config");
    if (config == NULL){
        exit(1);
    }
    iniciar_servidores(config);
    t_list* lista_de_procesos_con_ins = NULL;
    pthread_t hilo_kernel;
    pthread_create(&hilo_kernel,NULL, escuchar_kernel, NULL);
    pthread_t hilo_cpu;
    pthread_create(&hilo_cpu, NULL, escuchar_cpu, (void*)params_cpu);

    pthread_t hilo_io;
    pthread_create(&hilo_io, NULL, (void* (*)(void*)) esperar_clientes_IO, (void*) escucha_io );
    //(void* (*)(void*)) esperar_clientes_IO es casteo ya que pthread_create espera un void* con args void*
    // mientras q esperar cliente es voi* y espera un t_conexion_conexion escucha.
    config_destroy(config);
    pthread_join(hilo_cpu, NULL);
    pthread_join(hilo_kernel, NULL);
    free(escucha_cpu);
    free(escucha_kernel);
    list_destroy(lista_de_procesos_con_ins);
    return 0;
}