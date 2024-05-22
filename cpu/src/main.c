#include "main.h"

int socket_memoria;
int socket_kernel_d;
int socket_kernel_i;
t_conexion_escucha* oyente_dispatch;
t_conexion_escucha* oyente_interrupt;
int socket_servidor_d;
int socket_servidor_i;
volatile int hay_interrupcion = 0;
PCB* pcb;

sem_t semaforo_pc;
t_log* log_ciclo;
int pshared = 0; // No compartido entre procesos
unsigned int initial_value = 0;

void iniciar_servidores(t_config* config) {
    t_log* log_serv_dispatch = log_create("servidorDispatch.log", "CPU", false, LOG_LEVEL_TRACE);
    t_log* log_serv_interrupt = log_create("servidorInterrupt.log", "CPU", false, LOG_LEVEL_TRACE);

    socket_servidor_d = iniciar_servidor(config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH"), log_serv_dispatch);
    socket_servidor_i = iniciar_servidor(config_get_string_value(config, "PUERTO_ESCUCHA_INTERRUPT"), log_serv_interrupt);

    socket_kernel_d = esperar_cliente(socket_servidor_d, CPU);
    socket_kernel_i = esperar_cliente(socket_servidor_i, CPU);


    oyente_dispatch = malloc(sizeof(t_conexion_escucha));
    oyente_dispatch->socket_servidor = socket_kernel_d;
    oyente_dispatch->modulo = CPU;

    oyente_interrupt = malloc(sizeof(t_conexion_escucha));
    oyente_interrupt->socket_servidor = socket_kernel_i;
    oyente_interrupt->modulo = CPU;

}

void* escuchar_dispatch() {
    while (1) {
        PCB* pcb = recibir_pcb();
        realizar_ciclo_de_instruccion();
        free(pcb);
    }
    return NULL;
}

void *escuchar_interrupt() {
    while (1) {
        recibir_interrupcion();
    }
    return NULL;
}


int main(int argc, char* argv[]) {
    t_config* config = config_create("cpu.config");
    if (config == NULL) {
        exit(1);
    }

    sem_init(&semaforo_pc,pshared,initial_value);

    inicializar_registros_cpu();

    pthread_mutex_init(&mutexInterrupt, NULL);

    iniciar_servidores(config);

    socket_memoria = crear_conexion(config_get_string_value(config, "IP_MEMORIA"), config_get_string_value(config, "PUERTO_MEMORIA"), CPU);

    pthread_t threadEscuchaDispatch;
    pthread_create(&threadEscuchaDispatch, NULL, escuchar_dispatch, NULL);

    pthread_t threadEscuchaInterrupt;
    pthread_create(&threadEscuchaInterrupt, NULL, escuchar_interrupt, NULL);

    pthread_join(threadEscuchaDispatch, NULL);
    pthread_join(threadEscuchaInterrupt, NULL);

    config_destroy(config);
    liberar_conexion(socket_memoria);
    free(oyente_dispatch);
    free(oyente_interrupt);
    pthread_mutex_destroy(&mutexInterrupt);

    return 0;
}