#include <stdlib.h>
#include <stdio.h>
#include <utils/client.h>
#include <utils/server.h>
#include <utils/serializacion.h>

int conexion_memoria;

int block_size;
int block_count;
int delay_compactation;
char* path_base_dialfs;
t_bitarray* bitmap;


typedef struct {
    uint8_t operacion;
    char nombre_archivo[256];
    int registro_tamaño;
    int registro_direccion;
    int registro_puntero_archivo;
} solicitud_dialfs_t;

ssize_t recibir_solicitud(int socket, solicitud_dialfs_t* solicitud) {
    ssize_t bytes_recibidos = recv(socket, solicitud, sizeof(solicitud_dialfs_t), 0);
    if (bytes_recibidos < 0) {
        perror("Error recibiendo solicitud");
        return -1;
    }
    return bytes_recibidos;
}

void* obtener_datos_memoria(int direccion, int tamaño) {
    void* datos = malloc(tamaño);
    memset(datos, 0, tamaño);
    return datos;
}

void enviar_datos_memoria(int direccion, void* datos, int tamaño) {
    //armar
}



void iniciarInterfazGenerica(int socket, t_config* config, char* nombre){

    int tiempo_pausa = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");

    t_paquete* paquete = crear_paquete(CONEXION_IOGENERICA);
    agregar_string_a_paquete(paquete, nombre);
    enviar_paquete(paquete ,socket);
    eliminar_paquete(paquete);

    // esto esta unicamente para que no quede memoria colgada
    paquete = crear_paquete(CONEXION_IOGENERICA);
    enviar_paquete(paquete, conexion_memoria);

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
    
    while(1) {
        ssize_t reciv = recibir_operacion(socket);
        
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
}

void iniciarInterfazSTDOUT(int socket, t_config* config, char* nombre) {

    char* texto_completo = NULL;

    t_paquete* paquete = crear_paquete(CONEXION_STDOUT);
    agregar_string_a_paquete(paquete, nombre);
    enviar_paquete(paquete ,socket);
    eliminar_paquete(paquete);

    paquete = crear_paquete(CONEXION_STDOUT);
    enviar_paquete(paquete, conexion_memoria);
    eliminar_paquete(paquete);

    while(1) {
        ssize_t reciv = recibir_operacion(socket);

        if (reciv < 0) {
            exit(-1);
        }
        t_buffer* buffer = recibir_buffer(socket);
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
        enviar_paquete(paquete ,socket);
        eliminar_paquete(paquete);
    }
}

void inicializar_bitmap() {
    char* bitmap_path;
    sprintf(bitmap_path, "%s/bitmap.dat", path_base_dialfs);

    int fd = open(bitmap_path, O_RDWR);
    if (fd < 0) {
        perror("No se pudo abrir el archivo de bitmap");
        exit(EXIT_FAILURE);
    }

    struct stat stat_buf;
    fstat(fd, &stat_buf);

    void* bitarray = mmap(NULL, stat_buf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    bitmap = bitarray_create_with_mode(bitarray, stat_buf.st_size, LSB_FIRST);

    close(fd);
}
void compactar_filesystem() {
    usleep(delay_compactation * 1000);
}
void crear_archivo(char* nombre_archivo) {
    char* metadata_path;
    sprintf(metadata_path, "%s/%s", path_base_dialfs, nombre_archivo);

    FILE* metadata_file = fopen(metadata_path, "w");
    if (!metadata_file) {
        perror("No se pudo crear el archivo de metadata");
        exit(EXIT_FAILURE);
    }

    int bloque_inicial = -1;
    for (int i = 0; i < block_count; i++) {
        if (!bitarray_test_bit(bitmap, i)) {
            bitarray_set_bit(bitmap, i);
            bloque_inicial = i;
            break;
        }
    }

    if (bloque_inicial == -1) {
        perror("No hay bloques libres disponibles");
        fclose(metadata_file);
        exit(EXIT_FAILURE);
    }

    fprintf(metadata_file, "BLOQUE_INICIAL=%d\nTAMANIO_ARCHIVO=0\n", bloque_inicial);
    fclose(metadata_file);
}

void borrar_archivo(char* nombre_archivo) {
    char* metadata_path;
    sprintf(metadata_path, "%s/%s", path_base_dialfs, nombre_archivo);

    FILE* metadata_file = fopen(metadata_path, "r");
    if (!metadata_file) {
        perror("No se pudo abrir el archivo de metadata para borrar");
        exit(EXIT_FAILURE);
    }

    int bloque_inicial, tam_archivo;
    fscanf(metadata_file, "BLOQUE_INICIAL=%d\nTAMANIO_ARCHIVO=%d\n", &bloque_inicial, &tam_archivo);
    fclose(metadata_file);

    for (int i = 0; i < (tam_archivo + block_size - 1) / block_size; i++) {
        bitarray_clean_bit(bitmap, bloque_inicial + i);
    }

    remove(metadata_path);
}

void truncar_archivo(char* nombre_archivo, int nuevo_tamanio) {
    char* metadata_path;
    sprintf(metadata_path, "%s/%s", path_base_dialfs, nombre_archivo);

    FILE* metadata_file = fopen(metadata_path, "r+");
    if (!metadata_file) {
        perror("No se pudo abrir el archivo de metadata para truncar");
        exit(EXIT_FAILURE);
    }

    int bloque_inicial, tam_archivo;
    fscanf(metadata_file, "BLOQUE_INICIAL=%d\nTAMANIO_ARCHIVO=%d\n", &bloque_inicial, &tam_archivo);

    int bloques_necesarios = (nuevo_tamanio + block_size - 1) / block_size;
    int bloques_actuales = (tam_archivo + block_size - 1) / block_size;

    if (bloques_necesarios > bloques_actuales) {
        for (int i = bloques_actuales; i < bloques_necesarios; i++) {
            if (bitarray_test_bit(bitmap, bloque_inicial + i)) {
                compactar_filesystem();
                break;
            }
        }

        for (int i = bloques_actuales; i < bloques_necesarios; i++) {
            bitarray_set_bit(bitmap, bloque_inicial + i);
        }
    } else if (bloques_necesarios < bloques_actuales) {
        for (int i = bloques_necesarios; i < bloques_actuales; i++) {
            bitarray_clean_bit(bitmap, bloque_inicial + i);
        }
    }

    fseek(metadata_file, 0, SEEK_SET);
    fprintf(metadata_file, "BLOQUE_INICIAL=%d\nTAMANIO_ARCHIVO=%d\n", bloque_inicial, nuevo_tamanio);
    fclose(metadata_file);
}

void escribir_archivo(char* nombre_archivo, void* datos, int offset, int tam) {
    char* metadata_path;
    sprintf(metadata_path, "%s/%s", path_base_dialfs, nombre_archivo);

    FILE* metadata_file = fopen(metadata_path, "r+");
    if (!metadata_file) {
        perror("No se pudo abrir el archivo de metadata para escribir");
        exit(EXIT_FAILURE);
    }

    int bloque_inicial, tam_archivo;
    fscanf(metadata_file, "BLOQUE_INICIAL=%d\nTAMANIO_ARCHIVO=%d\n", &bloque_inicial, &tam_archivo);

    if (offset + tam > tam_archivo) {
        truncar_archivo(nombre_archivo, offset + tam);
    }

    int bloque_actual = bloque_inicial + (offset / block_size);
    int desplazamiento = offset % block_size;

    while (tam > 0) {
        int tam_a_escribir = (tam > block_size - desplazamiento) ? block_size - desplazamiento : tam;
        enviar_a_memoria_para_escribir(bloque_actual * block_size + desplazamiento, datos, tam_a_escribir, -1);

        datos += tam_a_escribir;
        tam -= tam_a_escribir;
        desplazamiento = 0;
        bloque_actual++;
    }

    fseek(metadata_file, 0, SEEK_SET);
    fprintf(metadata_file, "BLOQUE_INICIAL=%d\nTAMANIO_ARCHIVO=%d\n", bloque_inicial, (offset + tam > tam_archivo) ? offset + tam : tam_archivo);
    fclose(metadata_file);
}

void leer_archivo(char* nombre_archivo, void* buffer, int offset, int tam) {
    char* metadata_path;
    sprintf(metadata_path, "%s/%s", path_base_dialfs, nombre_archivo);

    FILE* metadata_file = fopen(metadata_path, "r");
    if (!metadata_file) {
        perror("No se pudo abrir el archivo de metadata para leer");
        exit(EXIT_FAILURE);
    }

    int bloque_inicial, tam_archivo;
    fscanf(metadata_file, "BLOQUE_INICIAL=%d\nTAMANIO_ARCHIVO=%d\n", &bloque_inicial, &tam_archivo);
    fclose(metadata_file);

    if (offset >= tam_archivo) {
        return;  // No hay nada que leer
    }

    if (offset + tam > tam_archivo) {
        tam = tam_archivo - offset;
    }
}

void iniciarInterfazDialFS(int socket, t_config* config, char* nombre){
    leer_configuracion(config);
    inicializar_bitmap();

    solicitud_dialfs_t solicitud;

    while (1) {
        ssize_t recibidos = recibir_solicitud(socket, &solicitud);

        if (recibidos <= 0) {
            if (recibidos < 0) perror("Error recibiendo solicitud");
            break;
        }

        switch (solicitud.operacion) {
            case IO_FS_CREATE:
                crear_archivo(solicitud.nombre_archivo);
                break;

            case IO_FS_DELETE:
                borrar_archivo(solicitud.nombre_archivo);
                break;

            case IO_FS_TRUNCATE:
                truncar_archivo(solicitud.nombre_archivo, solicitud.registro_tamaño);
                break;

            case IO_FS_WRITE:
                {
                    // Obtener datos desde memoria
                    void* datos = obtener_datos_memoria(solicitud.registro_direccion, solicitud.registro_tamaño);
                    escribir_archivo(solicitud.nombre_archivo, datos, solicitud.registro_puntero_archivo, solicitud.registro_tamaño);
                    free(datos);
                }
                break;

            case IO_FS_READ:
                {
                    // Crear buffer para lectura
                    void* buffer = malloc(solicitud.registro_tamaño);
                    leer_archivo(solicitud.nombre_archivo, buffer, solicitud.registro_puntero_archivo, solicitud.registro_tamaño);
                    enviar_datos_memoria(solicitud.registro_direccion, buffer, solicitud.registro_tamaño);
                    free(buffer);
                }
                break;

            default:
                fprintf(stderr, "Operación desconocida: %d\n", solicitud.operacion);
                break;
        }

        // Notificar al cliente que la operación ha sido completada (puedes implementar esta parte según tu protocolo)
        send(socket, "Operación completada", 21, 0);
    }

}

int main(int argc, char* argv[]) {

    t_config* nuevo_config = config_create(argv[2]);
    if (nuevo_config == NULL) {
        exit(1);
    }; 

    int conexion_kernel = crear_conexion(config_get_string_value(nuevo_config,"IP_KERNEL"), config_get_string_value(nuevo_config, "PUERTO_KERNEL"), IO);
    conexion_memoria = crear_conexion(config_get_string_value(nuevo_config, "IP_MEMORIA"), config_get_string_value(nuevo_config, "PUERTO_MEMORIA"), IO);

    char* tipo = config_get_string_value(nuevo_config,"TIPO_INTERFAZ");

    if(!strcmp(tipo,"GENERICA")){
        iniciarInterfazGenerica(conexion_kernel, nuevo_config, argv[1]);
    }

    if(!strcmp(tipo,"STDIN")){
        iniciarInterfazSTDIN(conexion_kernel, nuevo_config, argv[1]);
    }

    if (!strcmp(tipo, "STDOUT")) {
        iniciarInterfazSTDOUT(conexion_kernel, nuevo_config, argv[1]);
    }

    if (!strcmp(tipo, "DIALFS")) {
        iniciarInterfazDialFS(conexion_kernel, nuevo_config, argv[1]);
    }


    return 0;
}