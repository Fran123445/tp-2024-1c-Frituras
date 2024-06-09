#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<commons/log.h>
#include<commons/config.h>
#include<netdb.h>
#include<string.h>
#include<pthread.h>
#include <utils/serializacion.h>
#include <commons/bitarray.h>
#include "memoriaDeInstrucciones.h"



extern void* memoria_contigua;
void* iniciar_memoria(t_config* config);
int calcular_marcos (t_config* config)
