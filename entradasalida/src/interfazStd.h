#ifndef STD_H_
#define STD_H_

#include "main.h"


void pedir_contenido_memoria(uint32_t direccion_fisica, uint32_t tam, int  pid);
void* recibir_contenido_memoria_fs();
void* contenido_obtenido_de_memoria(uint32_t direccion_fisica, uint32_t tam, int  pid);
void enviar_a_memoria_para_escribir(uint32_t direccion_fisica, void* datos_a_escribir, uint32_t tam, int pid);
void iniciarInterfazSTDIN(t_config* config, char* nombre);
void iniciarInterfazSTDOUT(t_config* config, char* nombre);

#endif /* STD_H_ */