#include <utils/serializacion.h>





extern pthread_mutex_t mutex_log_memoria_io;
void esperar_clientes_IO(t_conexion_escucha* nueva_conexion, int tiempo_retardo, t_config* config);