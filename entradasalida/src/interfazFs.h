#ifndef FS_H_
#define FS_H_

#include "main.h"

extern uint8_t* bitmap;
extern int block_count;
extern int block_size;
extern int retraso_compactacion;

void* cargar_bitmap(char* path_base_dialfs);
void guardar_bitmap(char* path_base_dialfs);
int encontrar_bloque_libre();
void marcar_bloque(int bloque, int ocupado);
void crear_metadata(char* path, char* nombre_archivo, int bloque_inicial, int tamano_archivo);
void leer_metadata(char* path, char* nombre_archivo, int* bloque_inicial, int* tamano_archivo);
void compactar_fs();
void crear_archivo_en_dialfs(char* path, char* nombre_archivo, int tam);
void eliminar_archivo_en_dialfs(char* path,char* nombre_archivo);
void truncar_archivo_en_dialfs(char*path ,char* nombre_archivo, int nuevo_tamano, int retraso_compactacion);
void escribir_en_archivo_dialfs(char* path, char* nombre_archivo, char* texto);
void leer_desde_archivo_dialfs(char* path, char* nombre_archivo);
void crear_archivo_de_bloques(char* path_base_dialfs,char* nombre, int tam);
void iniciarInterfazDialFS(t_config* config, char* nombre);

void iniciarInterfazDialFS(t_config* config, char* nombre);


#endif /* FS_H_ */