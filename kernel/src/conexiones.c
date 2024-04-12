#include <stdlib.h>
#include <stdio.h>
#include <utils/client.h>
#include <utils/server.h>

t_config* nuevo_config = config_create("kernel.config");
	if (config == NULL) {
    exit(1);
}; 

iniciar_servidor(config_get_string_value(nuevo_config, "PUERTO_ESCUCHA"));
