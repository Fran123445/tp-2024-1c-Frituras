#ifndef CONS_H
#define CONS_H

#include <stdlib.h>
#include <stdio.h>
#include <commons/string.h>
#include <readline/readline.h>
#include <planificacion.h>
#include <planificadorLP.h>

// Solicita input por consola al usuario
void solicitarInput(void);

// Interpreta el comando ingresado y ejecuta la funcion correspondiente
void interpretarInput(char*);

// Imprime una lista de los procesos en ejecucion ordenados por estado
void listarProcesos(void);

// Itera a traves de un archivo de texto que contenga comandos del kernel y solicita que se interpreten 
void ejecutarScript(char*); 

#endif /* CONS_H */