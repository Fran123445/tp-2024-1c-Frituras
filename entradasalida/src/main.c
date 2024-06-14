#include <stdlib.h>
#include <stdio.h>
#include <utils/client.h>
#include <utils/server.h>
#include <utils/serializacion.h>

void iniciarInterfazGenerica(int socket, t_config* config, char* nombre){

    int tiempo_pausa = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");

    t_paquete* paquete = crear_paquete(CONEXION_IOGENERICA);
    agregar_string_a_paquete(paquete, nombre);
    enviar_paquete(paquete ,socket);
    eliminar_paquete(paquete);

    while (1) {
       ssize_t reciv = recibir_operacion(socket);

        if (reciv < 0) {
            exit(-1);
        }
        
        t_buffer* buffer = recibir_buffer(socket);
        int unidades_trabajo = buffer_read_int(buffer);
        sleep(tiempo_pausa * unidades_trabajo);

        t_paquete* paquete = crear_paquete(OPERACION_FINALIZADA);
        enviar_paquete(paquete ,socket);
        eliminar_paquete(paquete);

    }

}

void iniciarInterfazSTDIN(int socket, t_config* config, char* nombre) {
    char* ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    char* puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    int conexion_memoria = crear_conexion(ip_memoria, puerto_memoria, IO);

    char* texto;
    printf("Ingrese texto: ");
    fgets(texto, sizeof(texto), stdin);
    int texto_len = strlen(texto);

    int enviado = 0;

    while (enviado < texto_len) {
        ssize_t reciv = recibir_operacion(socket);
        if (reciv < 0) {
            exit(-1);
        }

        t_buffer* buffer = recibir_buffer(socket);
        uint32_t direccion_fisica = buffer_read_uint32(buffer);
        uint32_t tam = buffer_read_uint32(buffer);

        while (enviado < texto_len && tam > 0) {
            int tam_envio = (texto_len - enviado > tam) ? tam : texto_len - enviado;
            char* parte_texto = strndup(texto + enviado, tam_envio);
        
            enviar_a_memoria_para_escritura(direccion_fisica,parte_texto, tam_envio);

            free(parte_texto);
            enviado += tam_envio;

            direccion_fisica += tam_envio;

            tam -= tam_envio;  // Reducimos el tamaño restante de la página
        }

    }
    t_paquete* paquete = crear_paquete(OPERACION_FINALIZADA);
    enviar_paquete(paquete ,socket);
    eliminar_paquete(paquete);

}

void iniciarInterfazSTDOUT(int socket, t_config* config, char* nombre) {
    char* ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    char* puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    int conexion_memoria = crear_conexion(ip_memoria, puerto_memoria, IO);
    
    char* texto_completo = NULL;
    int texto_completo_len = 0;

    while (1) {
        ssize_t reciv = recibir_operacion(socket);

        if (reciv < 0) {
            exit(-1);
        }

        t_buffer* buffer = recibir_buffer(socket);
        uint32_t direccion_fisica = buffer_read_uint32(buffer);
        uint32_t tam = buffer_read_uint32(buffer);

        char* texto_pagina = (char*) contenido_obtenido_de_memoria(direccion_fisica, tam);
            
        // Concatenar la página recibida al texto completo
        texto_completo = realloc(texto_completo, texto_completo_len + strlen(texto_pagina) + 1);
        strcat(texto_completo, texto_pagina);
        texto_completo_len += strlen(texto_pagina);

        // Liberar recursos
        free(texto_pagina);

        eliminar_buffer(buffer);

    }   
    printf("STDOUT: %s\n", texto_completo);
    free(texto_completo);
    t_paquete* paquete = crear_paquete(OPERACION_FINALIZADA);
    enviar_paquete(paquete ,socket);
    eliminar_paquete(paquete);

}


int main(int argc, char* argv[]) {

    t_config* nuevo_config = config_create(argv[2]);
    if (nuevo_config == NULL) {
        exit(1);
    }; 

    int conexion_kernel = crear_conexion(config_get_string_value(nuevo_config,"IP_KERNEL"), config_get_string_value(nuevo_config, "PUERTO_KERNEL"), IO);

    char* tipo = config_get_string_value(nuevo_config,"TIPO_INTERFAZ");

    if(!strcmp(tipo,"GENERICA")){
        iniciarInterfazGenerica(conexion_kernel, nuevo_config, argv[1]);
    }

    if(!strcmp(tipo,"IO_STDIN_READ")){
        iniciarInterfazSTDIN(conexion_kernel, nuevo_config, argv[1]);
    }

    if (!strcmp(tipo, "IO_STDOUT_WRITE")) {
        iniciarInterfazSTDOUT(conexion_kernel, nuevo_config, argv[1]);
    }

    return 0;
}