#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <utils/server.h>
#include <pthread.h>
#include <semaphore.h>
#include "conexiones.h"
#include "estructuras.h"
#include "memoriaCPU.h"
#include "memoriaKernel.h"
#include "memoriaInstrucciones.h"

extern int socket_kernel;
extern int socket_cpu;
extern int socket_io;
void* escuchar_kernel();
void* escuchar_cpu();