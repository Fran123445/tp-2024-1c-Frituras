#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <utils/server.h>
#include <utils/client.h>
#include "instrucciones.h"
#include "cicloDeInstrucciones.h"

extern PCB* pcb;
extern int socket_memoria;
extern int socket_kernel_d;
extern int socket_kernel_i;
extern volatile int hay_interrupcion;