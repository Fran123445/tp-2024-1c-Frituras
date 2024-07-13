#ifndef FS_H_
#define FS_H_

#include "main.h"
#include "math.h"

extern t_bitarray* bitmap;
extern int block_count;
extern int block_size;
extern int retraso_compactacion;

void cargar_bitmap();
void guardar_bitmap();
int encontrar_bloque_libre();
void marcar_bloque(int bloque, int ocupado);
void crear_metadata(char* nombre_archivo, int bloque_inicial, int tamano_archivo, bool modificar);
void leer_metadata(char* nombre_archivo, int* bloque_inicial, int* tamano_archivo);
void compactar_fs();
char* rutacompleta(char* nombre_archivo);
void crear_archivo_en_dialfs(char* nombre_archivo, int tam);
void eliminar_archivo_en_dialfs(char* nombre_archivo);
void truncar_archivo_en_dialfs(char* nombre_archivo, int nuevo_tamano, int retraso_compactacion);
void escribir_en_archivo_dialfs(char* nombre_archivo, int direccion, int tamanio, int ubicacionPuntero, int pid);
void leer_desde_archivo_dialfs(char* nombre_archivo, int direccion, int tamanio, int ubicacionPuntero, int pid);
void iniciarInterfazDialFS(t_config* config, char* nombre);

typedef struct {
    int bloque_inicial;
    int tamano_archivo;
} MetadataArchivo;

#endif /* FS_H_ */