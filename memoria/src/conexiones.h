#include <utils/serializacion.h>



extern pthread_mutex_t mutex_log_memoria_io;
void funcion_io(int* socket_cliente);
void esperar_clientes_IO(t_conexion_escucha* nueva_conexion);