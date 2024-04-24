#ifndef CONS_H
#define CONS_H

#include <stdlib.h>
#include <stdio.h>
#include <commons/string.h>
#include <readline/readline.h>
#include <procesos.h>

void solicitarInput(void);
void interpretarInput(char*);
void listarProcesos(void);
void ejecutarScript(char*); 

#endif /* CONS_H */