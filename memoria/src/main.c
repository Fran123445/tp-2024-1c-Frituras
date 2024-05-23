#include "main.h"


int socket_kernel;
int socket_cpu;
int socket_serv_cpu;
int socket_serv_kernel;
t_conexion_escucha* escucha_cpu;
t_conexion_escucha* escucha_kernel;
t_config* config;

void iniciar_servidores(t_config* config){
    t_log* log_memoria_kernel = log_create("memoria_kernel", "Memoria",true, LOG_LEVEL_TRACE);
    t_log* log_memoria_cpu = log_create("memoria_cpu.log", "Memoria", true, LOG_LEVEL_TRACE);
    t_log* log_memoria_io = log_create("memoria_io.log", "Memoria", true, LOG_LEVEL_TRACE);

    socket_serv_kernel = iniciar_servidor(config_get_string_value(config, "PUERTO_ESCUCHA"),log_memoria_kernel);
    socket_serv_cpu = iniciar_servidor(config_get_string_value(config,"PUERTO_ESCUCHA"), log_memoria_cpu);

    socket_kernel = esperar_cliente(socket_serv_kernel, KERNEL);
    socket_cpu = esperar_cliente(socket_serv_cpu, CPU);

    escucha_cpu = malloc(sizeof(t_conexion_escucha));
    escucha_cpu->modulo=MEMORIA;
    escucha_cpu->socket_servidor=  socket_cpu;
    escucha_kernel= malloc(sizeof(t_conexion_escucha));
    escucha_kernel->socket_servidor=socket_cpu;
    escucha_kernel->modulo= MEMORIA;

}
void* escuchar_cpu(){
    int tiempo_retardo = config_get_int_value(config, "PUERTO_ESCUCHA");
    while(1){
        recibir_pc(socket_cpu);
        mandar_instruccion_cpu(socket_kernel,socket_cpu, tiempo_retardo);

    }
}
void* escuchar_kernel(){
    while(1){
        creacion_proceso_path(socket_kernel);
        abrir_archivo_path(socket_kernel);
    }
}
void* escuchar_io(){
    
}
int main(int argc, char *argv[]){

    t_config *config = config_create("memoria.config");
    if (config == NULL){
        exit(1);
    }
    iniciar_servidores(config);

    pthread_t hilo_kernel;
    pthread_create(&hilo_kernel,NULL, escuchar_kernel, NULL);
    pthread_t hilo_cpu;
    pthread_create(&hilo_cpu, NULL, escuchar_cpu, NULL);

    config_destroy(config);
    pthread_join(hilo_cpu, NULL);
    pthread_join(hilo_kernel, NULL);
    free(escucha_cpu);
    free(escucha_kernel);
    return 0;
}