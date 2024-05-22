#include <utils/serializacion.h>

int recibir_pc(int socketCPUDispatch);
void* mandar_instruccion_cpu(int socket_kernel, int socketCPUDispatch, int tiempo_retardo);
