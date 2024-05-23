#include <utils/serializacion.h>
#include <stdlib.h>
#include <string.h>
#include <commons/collections/list.h>
#include <estructuras.h>
void* creacion_proceso(int socket_kernel) {
    op_code cod_op = recibir_operacion(socket_kernel);
    if(cod_op == CREACION_PROCESO){
        proceso_memoria* proceso = malloc(sizeof(proceso_memoria));
        t_buffer* buffer = recibir_buffer(socket_kernel);
        int pid_proceso= buffer_read_int(buffer);
        proceso->pid=pid_proceso;
        char* path_proceso = buffer_read_string(buffer);
        proceso->path= path_proceso;
        liberar_buffer(buffer);
        return proceso;
    }
    return NULL;
}

t_list *abrir_archivo_path(int socket_kernel){
    proceso_memoria* proceso = creacion_proceso_path(socket_kernel);
    char* path = stdrup(proceso->path);
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
        free(buffer);
        return lista_pseudocodigo;
    } else{
        printf ("Error, path no valido");
        return NULL;
    }

}


