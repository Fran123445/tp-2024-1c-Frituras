#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/log.h>
#include <pthread.h>
#include <semaphore.h>
#include <stddef.h> 
#include <utils/pcb.h>
#include <utils/serializacion.h>
#include <utils/server.h>
#include <utils/client.h>
#include "instrucciones.h"
#include "interrupciones.h"
#include "cicloDeInstrucciones.h"

extern t_instruccion* instrucciones[4];
extern PCB* pcb;
extern int socket_memoria;
extern int socket_kernel_d;
extern int socket_kernel_i;
extern volatile int hay_interrupcion;