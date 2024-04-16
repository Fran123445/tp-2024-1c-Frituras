#include <conexiones.h>

int conexion;
char* ip;
char* puerto;

int escucharConexionesCpu(void) {

    t_config* nuevo_config = config_create("cpu.config");
        if (nuevo_config == NULL) {
        exit(1);
        }

    char* puertodispatch = config_get_string_value(nuevo_config, "PUERTO_ESCUCHA_DISPATCH");
    char* puertointerrupt = config_get_string_value(nuevo_config, "PUERTO_ESCUCHA_INTERRUPT");

	logger = log_create("ServidorCpu.log", "Servidor Cpu", 1, LOG_LEVEL_DEBUG);

    int server_di = iniciar_servidor(puertodispatch);
    int server_it = iniciar_servidor(puertointerrupt);

	log_info(logger, "Servidor listo para recibir al cliente");

    esperar_cliente(server_di);
	esperar_cliente(server_it);
    
    // Dispatch
    pthread_t thread;
    pthread_create(&thread,
						NULL,
						(void*) esperar_cliente,
						&server_di);
	pthread_detach(thread);
    
    // Interrupt
    pthread_t thread2;
    pthread_create(&thread2,
						NULL,
						(void*) esperar_cliente,
						&server_it);
	pthread_detach(thread2);

	return 0;
}


