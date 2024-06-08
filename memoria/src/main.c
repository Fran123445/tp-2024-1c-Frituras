#include<stdint.h>
#include "main.h"
#include "memoriaCPU.h"
#include "conexiones.h"
#include "memoriaContigua.h"
int socket_kernel= 0;
int socket_cpu = 0;
int socket_io = 0;
int socket_servidor_memoria;
void* memoria_contigua = 0;
int cant_marcos = 0;
t_conexion_escucha* escucha_cpu;
t_conexion_escucha* escucha_kernel;
t_conexion_escucha* escucha_io;
t_config* config;
t_parametros_cpu* params_cpu;



void iniciar_servidores(t_config* config){
    t_log* log_memoria = log_create("memoria.log", "Memoria",true, LOG_LEVEL_TRACE);

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

int main(int argc, char *argv[]){
    lista_de_procesos = list_create();

    config = config_create("memoria.config");
    if (config == NULL){
        exit(1);
    }
    iniciar_servidores(config);
    memoria_contigua = iniciar_memoria(config);
    cant_marcos = calcular_marcos(config);

    pthread_t hilo_kernel;
    pthread_create(&hilo_kernel,NULL, escuchar_kernel, NULL);
    pthread_t hilo_cpu;
    pthread_create(&hilo_cpu, NULL, escuchar_cpu, (void*)params_cpu);
    pthread_join(hilo_cpu, NULL);
    pthread_join(hilo_kernel, NULL);
    config_destroy(config);
    free(escucha_cpu);
    free(escucha_kernel);
    list_destroy(lista_de_procesos_con_ins);


    return 0;
}