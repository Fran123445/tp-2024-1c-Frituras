#include <conexiones.h>


int escucharConexiones(void) {
    t_config* nuevo_config = config_create("kernel.config");
    if (nuevo_config == NULL) {
        exit(1);
    }; 

    char* puerto = config_get_string_value(nuevo_config, "PUERTO_ESCUCHA");

	logger = log_create("ServidorKernel.log", "Servidor Kernel", 1, LOG_LEVEL_DEBUG);

    int server_fd = iniciar_servidor(puerto);
    
	log_info(logger, "Servidor listo para recibir al cliente");
	esperar_cliente(server_fd);

	return 0;
}

