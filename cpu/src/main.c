#include "main.h"

int socket_memoria;

int socket_kernel_d;
t_conexion_escucha* oyente_dispatch;
int socket_kernel_i;
t_conexion_escucha* oyente_interrupt;

volatile int hay_interrupcion;
PCB* pcb;

int main(int argc, char* argv[]) {
    
    t_config* config = config_create("cpu.config");
    if (config == NULL) {
        exit(1);
    };

    t_log* log_serv_dispatch = log_create("servidorDispatch", "CPU", false, LOG_LEVEL_TRACE);
    t_log* log_serv_interrupt = log_create("servidorInterrupt", "CPU", false, LOG_LEVEL_TRACE);

    socket_kernel_d = iniciar_servidor(config_get_string_value(config,"PUERTO_ESCUCHA_DISPATCH"),log_serv_dispatch);

    oyente_dispatch = malloc(sizeof(t_conexion_escucha));
    oyente_dispatch->socket_servidor = socket_kernel_d;
    oyente_dispatch->modulo = CPU;

    socket_kernel_i = iniciar_servidor(config_get_string_value(config,"PUERTO_ESCUCHA_INTERRUPT"),log_serv_interrupt);

    oyente_interrupt = malloc(sizeof(t_conexion_escucha));
    oyente_interrupt->socket_servidor = socket_kernel_i;
    oyente_interrupt->modulo = CPU;


    socket_memoria = crear_conexion(config_get_string_value(config, "IP_MEMORIA"), config_get_string_value(config, "PUERTO_MEMORIA"), CPU);;

    //Escuchar Conexiones
    pthread_t threadEscuchaDispatch;
    pthread_create(&threadEscuchaDispatch,
						NULL,
						(void*)threadEscuchaDispatch,
						oyente_dispatch);
    
    
    pthread_detach(threadEscuchaDispatch);

    pthread_t threadEscuchaInterrupt;
    pthread_create(&threadEscuchaInterrupt,
						NULL,
						(void*)threadEscuchaInterrupt,
						oyente_interrupt);
                
    pthread_detach(threadEscuchaInterrupt);

    //recibo el pcb del kernel
    pcb = recibir_pcb(servidor_dispatch);
    
    
    config_destroy(config);
    liberar_conexion(socket_memoria);
    free(oyente_dispatch);
    free(oyente_interrupt);

    return 0;
}
