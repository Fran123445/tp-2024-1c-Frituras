#include <utils/serializacion.h>
#include <stdlib.h>
#include <string.h>
#include <commons/collections/list.h>
#include <estructuras.h>

t_tipoInstruccion obtener_tipo_instruccion (const* char ins_char);
t_instruccion* volver_char_a_instruccion(char* linea);