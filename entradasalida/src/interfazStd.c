#include "interfazStd.h"


void pedir_contenido_memoria(uint32_t direccion_fisica, uint32_t tam, int  pid){
    t_paquete* paquete = crear_paquete(ACCESO_ESPACIO_USUARIO_LECTURA);
    agregar_uint32_a_paquete(paquete, direccion_fisica);
    agregar_uint32_a_paquete(paquete, tam);
    agregar_int_a_paquete(paquete, pid);
    enviar_paquete(paquete, conexion_memoria);
    eliminar_paquete(paquete);
}

void* recibir_contenido_memoria(){
    op_code cod_op = recibir_operacion(conexion_memoria);
    if(cod_op == ACCESO_ESPACIO_USUARIO_LECTURA){
        t_buffer* buffer = recibir_buffer(conexion_memoria);
        void* data = malloc(read_buffer_tamanio(buffer)+1);
        buffer_read(buffer, data);
        liberar_buffer(buffer);
        return data;
    }
    return NULL;
}

void* contenido_obtenido_de_memoria(uint32_t direccion_fisica, uint32_t tam, int  pid){
    pedir_contenido_memoria(direccion_fisica, tam, pid);
    void* contenido_leido = recibir_contenido_memoria();  //le pido a memoria el contenido de la pagina
    // void* puntero_al_dato_leido = &contenido_leido; Me parece que esto ya no hace falta, veremos si hay seg. fault o no
    return contenido_leido;
}

void enviar_a_memoria_para_escribir(uint32_t direccion_fisica, void* datos_a_escribir, uint32_t tam, int pid){
    t_paquete* paquete = crear_paquete(ACCESO_ESPACIO_USUARIO_ESCRITURA);
    agregar_uint32_a_paquete(paquete, direccion_fisica);
    agregar_uint32_a_paquete(paquete, tam);
    agregar_int_a_paquete(paquete,pid);
    agregar_a_paquete(paquete, datos_a_escribir,tam);
    enviar_paquete(paquete, conexion_memoria);
    eliminar_paquete(paquete);
}

void iniciarInterfazSTDIN(t_config* config, char* nombre){
    
    t_paquete* paquete = crear_paquete(CONEXION_STDIN);
    agregar_string_a_paquete(paquete, nombre);
    enviar_paquete(paquete, conexion_kernel);
    eliminar_paquete(paquete);

    paquete = crear_paquete(CONEXION_STDIN);
    enviar_paquete(paquete, conexion_memoria);
    eliminar_paquete(paquete);

    while(1) {
        ssize_t reciv = recibir_operacion(conexion_kernel);
        
        if (reciv < 0) {
            exit(-1);
        }

        char* texto = readline(">");

        int enviado = 0;
        t_buffer* buffer = recibir_buffer(conexion_kernel);
        int pid = buffer_read_int(buffer);


        while (buffer->size > 0) {
            uint32_t direccion_fisica = buffer_read_uint32(buffer);
            uint32_t tam = buffer_read_uint32(buffer);

            char* parte_texto = strndup(texto + enviado, tam);
        
            enviar_a_memoria_para_escribir(direccion_fisica,parte_texto, tam, pid);

            free(parte_texto);
            enviado += tam;
        }

        liberar_buffer(buffer);

        t_paquete* paquete = crear_paquete(OPERACION_FINALIZADA);
        enviar_paquete(paquete ,conexion_kernel);
        eliminar_paquete(paquete);
    }
}

void iniciarInterfazSTDOUT(t_config* config, char* nombre){

    char* texto_completo = NULL;

    t_paquete* paquete = crear_paquete(CONEXION_STDOUT);
    agregar_string_a_paquete(paquete, nombre);
    enviar_paquete(paquete, conexion_kernel);
    eliminar_paquete(paquete);

    paquete = crear_paquete(CONEXION_STDOUT);
    enviar_paquete(paquete, conexion_memoria);
    eliminar_paquete(paquete);

    while(1) {
        ssize_t reciv = recibir_operacion(conexion_kernel);

        if (reciv < 0) {
            exit(-1);
        }
        t_buffer* buffer = recibir_buffer(conexion_kernel);
        int pid = buffer_read_int(buffer);

        while (buffer->size > 0) {
            uint32_t direccion_fisica = buffer_read_uint32(buffer);
            uint32_t tam = buffer_read_uint32(buffer);

            texto_completo = (char*) contenido_obtenido_de_memoria(direccion_fisica, tam, pid);
            texto_completo[tam+1] = '\0';
                    
            printf("%s", texto_completo);
        }

        printf("\n");
        free(texto_completo);
        
        paquete = crear_paquete(OPERACION_FINALIZADA);
        enviar_paquete(paquete ,conexion_kernel);
        eliminar_paquete(paquete);
    }
}