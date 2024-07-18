#include "main.h"
#include "instrucciones.h"
#include "interrupciones.h"
#include "cicloDeInstrucciones.h"
#include "mmu.h"
#include "algoritmosSustitucion.h"

int socket_memoria;

int socket_kernel_d;
int socket_kernel_i;

int socket_servidor_d;
int socket_servidor_i;

t_log* log_cpu;

volatile int hay_interrupcion = 0;

PCB* pcb;

char* algoritmoSustitucionTLB;
int cant_entradas_TLB;
int tamanio_pagina; 

pthread_mutex_t mutexPCB;
int hayPCB = 0;

void iniciar_servidores(t_config* config) {
    t_log* log_serv_dispatch = log_create("servidorDispatch.log", "CPU", true, LOG_LEVEL_TRACE);
    t_log* log_serv_interrupt = log_create("servidorInterrupt.log", "CPU", true, LOG_LEVEL_TRACE);

    socket_servidor_d = iniciar_servidor(config_get_string_value(config, "PUERTO_ESCUCHA_DISPATCH"), log_serv_dispatch);
    socket_servidor_i = iniciar_servidor(config_get_string_value(config, "PUERTO_ESCUCHA_INTERRUPT"), log_serv_interrupt);
    
    socket_kernel_d = esperar_cliente(socket_servidor_d, CPU);
    socket_kernel_i = esperar_cliente(socket_servidor_i, CPU);

    log_destroy(log_serv_dispatch);
    log_destroy(log_serv_interrupt);
}

void* escuchar_dispatch() {
    while (1) {
        pcb = recibir_pcb();
        if(!pcb) break;
        hayPCB = 1;
        
        realizar_ciclo_de_instruccion();

        pthread_mutex_lock(&mutexPCB);
        liberar_pcb(pcb);
        hayPCB = 0;
        pthread_mutex_unlock(&mutexPCB);
    }
    return NULL;
}

void *escuchar_interrupt() {
    while (1) {
        if(recibir_interrupcion()) break;
    }
    return NULL;
}

int recibir_tamanio_pagina(){
    op_code cod_op = recibir_operacion(socket_memoria);
    if(cod_op == ENVIO_TAMANIO_PAGINA){
        t_buffer* buffer = recibir_buffer(socket_memoria);
        int data = buffer_read_int(buffer);
        liberar_buffer(buffer);
        return data;
    }
    return -1;
}

void liberar_estructuras_TLB(){
    // Vacío la cola FIFO y las entradas que también coincidan con la TLB, para evitar leaks
    if (strcmp(algoritmoSustitucionTLB, "FIFO") == 0) {
        while (!queue_is_empty(cola_FIFO)) {
            entrada_TLB* elemento_a_sacar = queue_pop(cola_FIFO);
            list_remove_element(TLB, elemento_a_sacar);
            free(elemento_a_sacar);
        }
        queue_destroy(cola_FIFO);
    }
    // Vacío la cola LRU y las entradas que coincidan con la TLB
    else if (strcmp(algoritmoSustitucionTLB, "LRU") == 0) {
        while (list_size(estructura_LRU) > 0) {
            entrada_TLB* elemento_a_sacar = list_get(estructura_LRU, 0);
            list_remove_element(TLB, elemento_a_sacar);
            list_remove_element(estructura_LRU, elemento_a_sacar);
            free(elemento_a_sacar);
        }
        list_destroy(estructura_LRU);
    }

    // No estoy seguro si deberían quedar elementos en la TLB, pero si quedaran, se eliminan.
    while (list_size(TLB) > 0) {
        entrada_TLB* elemento_a_sacar = list_get(TLB, 0);
        list_remove_element(TLB, elemento_a_sacar);
        free(elemento_a_sacar);
    }
    list_destroy(TLB);
}


int main(int argc, char* argv[]) {
    
    t_config* config = config_create(argv[1]);
    if (config == NULL) {
        exit(1);
    }

    log_cpu = log_create("Cpu.log", "CPU", true, LOG_LEVEL_INFO);

    cant_entradas_TLB = config_get_int_value(config, "CANTIDAD_ENTRADAS_TLB");

    if(cant_entradas_TLB !=0){
        TLB = list_create();
        algoritmoSustitucionTLB = config_get_string_value(config, "ALGORITMO_TLB");

        if(strcmp(algoritmoSustitucionTLB, "FIFO") == 0){
            cola_FIFO = queue_create();
        }
        else if(strcmp(algoritmoSustitucionTLB, "LRU") == 0){
            estructura_LRU = list_create();
        }
    }


    // Inicialización del mutex
    pthread_mutex_init(&mutexInterrupt, NULL);
    pthread_mutex_init(&mutexPCB, NULL);

    // Inicialización de los servidores 
    iniciar_servidores(config);    

    int a;
    recv(socket_kernel_d, &a, sizeof(int), MSG_WAITALL);
    socket_memoria = crear_conexion(config_get_string_value(config, "IP_MEMORIA"), config_get_string_value(config, "PUERTO_MEMORIA"), CPU);
    
    // Todo lo relacionado con paginación
    tamanio_pagina = recibir_tamanio_pagina();

    // Hilos encargados del ciclo de instrucciones
    pthread_t threadEscuchaDispatch;
    pthread_create(&threadEscuchaDispatch, NULL, escuchar_dispatch, NULL);

    pthread_t threadEscuchaInterrupt;
    pthread_create(&threadEscuchaInterrupt, NULL, escuchar_interrupt, NULL);

    pthread_join(threadEscuchaDispatch, NULL);
    pthread_join(threadEscuchaInterrupt, NULL);

    // Liberar la memoria
    log_destroy(log_cpu);
    liberar_conexion(socket_memoria);
    pthread_mutex_destroy(&mutexInterrupt);

    if(cant_entradas_TLB !=0){
        liberar_estructuras_TLB();
    }

    config_destroy(config);

    return 0;
}