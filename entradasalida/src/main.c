#include <stdlib.h>
#include <stdio.h>
#include <utils/client.h>
#include <utils/server.h>
#include <utils/serializacion.h>

int conexion_memoria;

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

void pedir_contenido_memoria(uint32_t direccion_fisica, uint32_t tam, uint32_t  pid){
    t_paquete* paquete = crear_paquete(ACCESO_ESPACIO_USUARIO_LECTURA);
    agregar_uint32_a_paquete(paquete, direccion_fisica);
    agregar_uint32_a_paquete(paquete, tam);
    agregar_uint32_a_paquete(paquete, pid);
    enviar_paquete(paquete, conexion_memoria);
    eliminar_paquete(paquete);
}

void* recibir_contenido_memoria(){
    op_code cod_op = recibir_operacion(conexion_memoria);
    if(cod_op == ACCESO_ESPACIO_USUARIO_LECTURA){
        t_buffer* buffer = recibir_buffer(conexion_memoria);
        void* data;
        buffer_read(buffer, data);
        liberar_buffer(buffer);
        return data;
    }
    return NULL;
}

void* contenido_obtenido_de_memoria(uint32_t direccion_fisica, uint32_t tam, uint32_t  pid){
    pedir_contenido_memoria(direccion_fisica, tam, pid);
    void* contenido_leido = recibir_contenido_memoria();  //le pido a memoria el contenido de la pagina
    // void* puntero_al_dato_leido = &contenido_leido; Me parece que esto ya no hace falta, veremos si hay seg. fault o no
    return contenido_leido;
}

void enviar_a_memoria_para_escribir(uint32_t direccion_fisica, void* datos_a_escribir, uint32_t tam, int pid)  {
    t_paquete* paquete = crear_paquete(ACCESO_ESPACIO_USUARIO_ESCRITURA);
    agregar_uint32_a_paquete(paquete, direccion_fisica);
    agregar_uint32_a_paquete(paquete, tam);
    agregar_int_a_paquete(paquete,pid);
    agregar_a_paquete(paquete, datos_a_escribir,tam);
    enviar_paquete(paquete, conexion_memoria);
    eliminar_paquete(paquete);
}

void iniciarInterfazSTDIN(int socket, t_config* config, char* nombre) {
    
    char* texto;
    printf("Ingrese texto: ");
    if (fgets(texto, sizeof(texto), stdin) == NULL) {
        fprintf(stderr, "Error texto usuario\n");
        exit(-1);
    }

    int texto_len = strlen(texto);

    if (texto_len > 0 && texto[texto_len - 1] == '\n') {
        texto[--texto_len] = '\0';  // Elimina el salto de linea final (si existe)
    }

    int enviado = 0;

    while (enviado < texto_len) {
        ssize_t reciv = recibir_operacion(socket);
        if (reciv < 0) {
            exit(-1);
        }

        t_buffer* buffer = recibir_buffer(socket);
        uint32_t direccion_fisica = buffer_read_uint32(buffer);
        uint32_t tam = buffer_read_uint32(buffer);
        int pid = buffer_read_int(buffer);

        while (enviado < texto_len && tam > 0) {
            int tam_envio = (texto_len - enviado > tam) ? tam : texto_len - enviado;
            char* parte_texto = strndup(texto + enviado, tam_envio);
        
            enviar_a_memoria_para_escribir(direccion_fisica,parte_texto, tam_envio,pid);

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

    char* texto_completo = NULL;

    ssize_t reciv = recibir_operacion(socket);

    if (reciv < 0) {
        exit(-1);
    }
    t_buffer* buffer = recibir_buffer(socket);
    uint32_t direccion_fisica = buffer_read_uint32(buffer);
    uint32_t tam = buffer_read_uint32(buffer);
    uint32_t  pid = buffer_read_uint32(buffer);

    texto_completo = (char*) contenido_obtenido_de_memoria(direccion_fisica, tam, pid);
            
    printf("STDOUT: %s\n", texto_completo);
    free(texto_completo);
    
    t_paquete* paquete = crear_paquete(OPERACION_FINALIZADA);
    enviar_paquete(paquete ,socket);
    eliminar_paquete(paquete);

}


int main(int argc, char* argv[]) {

    t_config* nuevo_config = config_create("entradasalida.config");
    if (nuevo_config == NULL) {
        exit(1);
    }; 

    int conexion_kernel = crear_conexion(config_get_string_value(nuevo_config,"IP_KERNEL"), config_get_string_value(nuevo_config, "PUERTO_KERNEL"), IO);
    conexion_memoria = crear_conexion(config_get_string_value(nuevo_config, "IP_MEMORIA"), config_get_string_value(nuevo_config, "PUERTO_MEMORIA"), IO);

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