#include<stdint.h>
#include "main.h"
#include "memoriaCPU.h"
#include "conexiones.h"
int socket_kernel= 0;
int socket_cpu = 0;
int socket_io = 0;
int socket_servidor_memoria;
t_conexion_escucha* escucha_cpu;
t_conexion_escucha* escucha_kernel;
t_conexion_escucha* escucha_io;
t_config* config;
t_parametros_cpu* params_cpu;

void iniciar_servidores(t_config* config){
    t_log* log_memoria = log_create("memoria_kernel", "Memoria",true, LOG_LEVEL_TRACE);

    socket_servidor_memoria = iniciar_servidor(config_get_string_value(config, "PUERTO_ESCUCHA"),log_memoria);

    escucha_io= malloc(sizeof(t_conexion_escucha));
    escucha_io->modulo= MEMORIA;
    escucha_io->socket_servidor= socket_servidor_memoria;

    socket_kernel = esperar_cliente(socket_servidor_memoria, MEMORIA);
    socket_cpu = esperar_cliente(socket_servidor_memoria, MEMORIA);

    pthread_t hilo_io;
    pthread_create(&hilo_io, NULL, (void*) esperar_clientes_IO, escucha_io );

}

void* escuchar_cpu(void* argumento){
    t_parametros_cpu* params_cpu = (t_parametros_cpu*)argumento;
    //params_cpu usado para pasar los parámetros al hilo sin problema. ignorar warning.
    int tiempo_retardo = config_get_int_value(config, "RETARDO_RESPUESTA");
    while(1){
        mandar_instruccion_cpu(socket_kernel,socket_cpu, tiempo_retardo);
    }
}
void* escuchar_kernel(){
    while(1){
        abrir_archivo_path(socket_kernel);
    }
}
//void* escuchar_io(){}

int main(int argc, char *argv[]){
    lista_de_procesos_con_ins = list_create();

    config = config_create("memoria.config");
    if (config == NULL){
        exit(1);
    }
    iniciar_servidores(config);
    pthread_t hilo_kernel;
    pthread_create(&hilo_kernel,NULL, escuchar_kernel, NULL);
    pthread_t hilo_cpu;
    pthread_create(&hilo_cpu, NULL, escuchar_cpu, (void*)params_cpu);

    //(void* (*)(void*)) esperar_clientes_IO es casteo ya que pthread_create espera un void* con args void*
    // mientras q esperar cliente es voi* y espera un t_conexion_conexion escucha.
    pthread_join(hilo_cpu, NULL);
    pthread_join(hilo_kernel, NULL);
    config_destroy(config);
    free(escucha_cpu);
    free(escucha_kernel);
    list_destroy(lista_de_procesos_con_ins);


    return 0;
}