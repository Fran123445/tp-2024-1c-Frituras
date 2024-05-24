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

extern pthread_mutex_t mutex_lista_de_procesos_con_ins;
char* obtener_instruccion(int socket_kernel, int pc, int pid);