#include <utils/serializacion.h>
#include <stdlib.h>
#include <string.h>
#include <commons/collections/list.h>

void* creacion_proceso_path(int socket_kernel) {
    op_code cod_op = recibir_operacion(socket_kernel);
    if(cod_op == CREACION_PROCESO){
        t_buffer* buffer = recibir_buffer(socket_kernel);
        char* path = buffer_read_string(buffer);
        liberar_buffer(buffer);
        return path;
    }
    return NULL;
}
int creacion_proceso_pid(int socket_kernel){
    op_code cod_op = recibir_operacion(socket_kernel);
    if(cod_op == CREACION_PROCESO){
        t_buffer* buffer= recibir_buffer(socket_kernel);
        int pid = buffer_read_int(buffer);
        liberar_buffer(buffer);
        return pid;
    } return -1;
}
t_list *abrir_archivo_path(int socket_kernel){
    char* path = creacion_proceso_path(socket_kernel);
    if (path != NULL){
        FILE *file = fopen(path,"r");
        if (file == NULL){
            fprintf(stderr, "Archivo vacio");
            free(path);
        }
        t_list *lista_pseudocodigo = list_create();
        char buffer[1000];
        while(fgets(buffer, sizeof(buffer),file)){
         list_add(lista_pseudocodigo, strdup(buffer));
        }
        fclose(file);
        free(path);
        return lista_pseudocodigo;
    } else{
        printf ("Error, path no valido");
        return NULL;
    }

}


