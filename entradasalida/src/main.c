#include <stdlib.h>
#include <stdio.h>
#include <utils/client.h>
#include <utils/server.h>
#include <utils/serializacion.h>
//Falta agregar hilos para diferentes conexiones 
typedef struct {
    char* nombre;
    int unidades_trabajo;
} t_interfaz_generica;
typedef struct {
    char* nombre;
} t_interfaz_stdin;
typedef struct {
    char* nombre;
} t_interfaz_stdout;

void iniciarInterfazGenerica(int socket, t_config* config, char* nombre){

    t_interfaz_generica interfaz;

    int tiempo_pausa = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");

    interfaz.nombre = nombre;

    t_paquete* paquete = crear_paquete();
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

        t_paquete* paquete = crear_paquete();
        paquete->codigo_operacion = OPERACION_FINALIZADA;
        enviar_paquete(paquete ,socket);
        eliminar_paquete(paquete);

    }

}

void iniciarInterfazSTDIN(int socket, t_config* config, char* nombre) {
    t_interfaz_stdin interfaz;

    interfaz.nombre = nombre;

    char* ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    char* puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    int conexion_memoria = crear_conexion(ip_memoria, puerto_memoria, IO);

    while (1) {
        ssize_t reciv = recibir_operacion(socket);

        if (reciv < 0) {
            exit(-1);
        }

        t_buffer* buffer = recibir_buffer(socket);
        int direccion_fisica = buffer_read_int(buffer);

        char* texto;
        printf("Ingrese texto: ");
        fgets(texto, sizeof(texto), stdin);

        t_paquete* paquete = crear_paquete();
        agregar_int_a_paquete(paquete, direccion_fisica);
        agregar_string_a_paquete(paquete, texto);
        enviar_paquete(paquete, conexion_memoria);
        eliminar_paquete(paquete);

        t_paquete* paquete_respuesta = crear_paquete();
        paquete_respuesta->codigo_operacion = OPERACION_FINALIZADA;
        enviar_paquete(paquete_respuesta, socket);
        eliminar_paquete(paquete_respuesta);
    }
}


void iniciarInterfazSTDOUT(int socket, t_config* config, char* nombre) {

    t_interfaz_stdout interfaz;

    interfaz.nombre = nombre;

    int tiempo_pausa = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");
    char* ip_memoria = config_get_string_value(config, "IP_MEMORIA");
    char* puerto_memoria = config_get_string_value(config, "PUERTO_MEMORIA");
    int conexion_memoria = crear_conexion(ip_memoria, puerto_memoria, IO);

    while (1) {
        ssize_t reciv = recibir_operacion(socket);

        if (reciv < 0) {
            exit(-1);
        }

        t_buffer* buffer = recibir_buffer(socket);
        int direccion_fisica = buffer_read_int(buffer);

        t_paquete* paquete_memoria = crear_paquete();
        agregar_int_a_paquete(paquete_memoria, direccion_fisica);
        enviar_paquete(paquete_memoria, conexion_memoria);
        eliminar_paquete(paquete_memoria);

        t_buffer* buffer_memoria = recibir_buffer(conexion_memoria);
        char* texto = buffer_read_string(buffer_memoria);

        printf("STDOUT: %s\n", texto);
        free(texto);
        eliminar_buffer(buffer_memoria);

    
        t_paquete* paquete_respuesta = crear_paquete();
        paquete_respuesta->codigo_operacion = OPERACION_FINALIZADA;
        enviar_paquete(paquete_respuesta, socket);
        eliminar_paquete(paquete_respuesta);
    }
}


int main(int argc, char* argv[]) {

    t_config* nuevo_config = config_create(argv[2]);
    if (nuevo_config == NULL) {
        exit(1);
    }; 

    int conexion_kernel = crear_conexion(config_get_string_value(nuevo_config,"IP_KERNEL"), config_get_string_value(nuevo_config, "PUERTO_KERNEL"), IO);

    char* tipo = config_get_string_value(nuevo_config,"TIPO_INTERFAZ");

    if(!strcmp(tipo,"IO_GEN_SLEEP")){
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