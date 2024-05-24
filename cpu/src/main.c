#include "main.h"

int socket_memoria;

int socket_kernel_d;
int socket_kernel_i;

int socket_servidor_d;
int socket_servidor_i;

volatile int hay_interrupcion = 0;
PCB* pcb;

t_instruccion* sum;
t_instruccion* set1;
t_instruccion* set2;
t_instruccion* exitt; 

t_instruccion* instrucciones[4];

void iniciar_servidores(t_config* config) {
    t_log* log_serv_dispatch = log_create("servidorDispatch.log", "CPU", false, LOG_LEVEL_TRACE);
    t_log* log_serv_interrupt = log_create("servidorInterrupt.log", "CPU", false, LOG_LEVEL_TRACE);

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

    pthread_mutex_init(&mutexInterrupt, NULL);

    sum->tipo = iSUM;
    sum->sizeArg1 = NULL;
    sum -> arg1 = pcb->registros.AX;
    sum->sizeArg2 = NULL;    
    sum -> arg2 = pcb->registros.BX;
    sum->sizeArg3 = NULL;
    sum->arg3 = NULL;                        
    sum->interfaz = NULL;        
    sum->archivo = NULL;    

    set1->tipo = iSET;
    set1->sizeArg1 = NULL;
    set1 -> arg1 = pcb->registros.AX;
    set1->sizeArg2 = NULL;    
    set1-> arg2 = 2;
    set1->sizeArg3 = NULL;
    set1->arg3 = NULL;                        
    set1->interfaz = NULL;        
    set1->archivo = NULL;   

    set2->tipo = iSET;
    set2->sizeArg1 = NULL;
    set2 -> arg1 = pcb->registros.BX;
    set2->sizeArg2 = NULL;    
    set2-> arg2 = 3;
    set2->sizeArg3 = NULL;
    set2->arg3 = NULL;                        
    set2->interfaz = NULL;        
    set2->archivo = NULL;  

    exitt->tipo = iEXIT;
    exitt->sizeArg1 = NULL;
    exitt -> arg1 = NULL;
    exitt->sizeArg2 = NULL;    
    exitt-> arg2 = NULL;
    exitt->sizeArg3 = NULL;
    exitt->arg3 = NULL;                        
    exitt->interfaz = NULL;        
    exitt->archivo = NULL;  

    instrucciones[0] = set1;
    instrucciones[1] = set2;
    instrucciones[2] = sum;
    instrucciones[3] = exitt;


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
    pthread_mutex_destroy(&mutexInterrupt);

    return 0;
}