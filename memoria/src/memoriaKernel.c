#include <utils/serializacion.h>
#include <stdlib.h>
#include <string.h>

void* creacionProceso(int socket_kernel) {
    op_code cod_op = recibir_operacion(socket_kernel);
    if(cod_op == CREACION_PROCESO){
        t_buffer* buffer = recibir_buffer(socket_kernel);
        char* path = buffer_read_string(buffer);
        liberar_buffer(buffer);
        return path;
    }
}

void abrir_archivo_path(int socket_kernel){
    char* path = creacionProceso(socket_kernel);
    if (path != NULL){
        FILE *file = fopen(path,"r");
        if (file == NULL){
            fprintf(stderr, "Archivo vacio");
            free(path);
            return;
        }
        char archivo [1000];
        while(fgets(archivo, sizeof(archivo),file)){
            printf("%s",archivo);
        }
        fclose(file);
        free(path);
    } else{
        printf ("Error, path no valido");
    }

}


