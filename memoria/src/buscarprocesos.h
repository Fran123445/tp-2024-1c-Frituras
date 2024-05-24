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
#include <semaphore.h>
#include "estructuras.h"
#include "main.h"

extern pthread_mutex_t mutex_lista_procesos;

t_proceso* hallar_proceso(int PID);