#include <stdlib.h>
#include <string.h>
#include <commons/collections/list.h>
#include <utils/serializacion.h>
#include <utils/pcb.h>
#include<unistd.h>
#include<netdb.h>
#include<pthread.h>
#include <semaphore.h>
#include "estructuras.h"
#include "memoriaKernel.h"

t_proceso_memoria* hallar_proceso(int PID);
void destruir_proceso(t_proceso_memoria* proceso);
void eliminar_proceso_de_lista_de_procesos(int PID);
char* obtener_instruccion(int socket_kernel, int pc, int pid);



