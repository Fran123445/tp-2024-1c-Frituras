#include <utils/serializacion.h>
#include <stdlib.h>
#include <commons/collections/list.h>
#include <pthread.h>
#include "estructuras.h"

extern pthread_mutex_t mutex_lista_de_procesos_con_ins;
t_tipoInstruccion obtener_tipo_instruccion (char* ins_char);
t_instruccion* volver_char_a_instruccion(char* linea);
registrosCPU string_a_registro(const char* registro);
t_instruccion* obtener_instruccion(int socket_kernel, int pc, int pid);
t_instruccion* crear_instruccion(t_tipoInstruccion tipo_instruccion, int sizeArg, void* arg1, int sizeArg2, void* arg2, int sizeArg3, void* arg3, char* interfaz, char* archivo);
t_list* dividir_cadena_en_tokens(const char* linea);
t_proceso* hallar_proceso(int PID);
