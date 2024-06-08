#include <utils/serializacion.h>
#include "estructuras.h"

t_proceso_memoria* recibir_proceso_cpu(int socketCPUDispatch);
void mandar_instruccion_cpu(int socket_kernel, int socket_cpu, int tiempo_retardo);
