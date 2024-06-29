#include <stdlib.h>
#include <stdio.h>
#include <utils/client.h>
#include <utils/server.h>
#include <utils/serializacion.h>

int conexion_memoria;

uint8_t* bitmap;
int block_count;
int block_size;
int retraso_compactacion;

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
 //BITMAP
void cargar_bitmap(char* path_base_dialfs) {
    char bitmap_path[strlen(path_base_dialfs) + strlen("/bitmap.dat") + 1];
    sprintf(bitmap_path, "%s/bitmap.dat", path_base_dialfs);

    FILE* file = fopen(bitmap_path, "rb");
    if (!file) {
        file = fopen(bitmap_path, "wb");
        if (!file) {
            perror("Error creando bitmap.dat");
            exit(EXIT_FAILURE);
        }

        int bitmap_size = (block_count + 7) / 8;
        bitmap = malloc(bitmap_size);
        if (!bitmap) {
            perror("Error de memoria");
            exit(EXIT_FAILURE);
        }
        memset(bitmap, 0, bitmap_size);
        fwrite(bitmap, bitmap_size, 1, file);
        fclose(file);
    } else {
        // Si el archivo existe lo cargo en memoria
        fseek(file, 0, SEEK_END);
        long size = ftell(file);
        fseek(file, 0, SEEK_SET);
        bitmap = malloc(size);
        if (!bitmap) {
            perror("Error de memoria");
            exit(EXIT_FAILURE);
        }
        fread(bitmap, size, 1, file);
        fclose(file);
    }

    printf("Bitmap cargado exitosamente desde %s.\n", bitmap_path);
}

void guardar_bitmap(char* path_base_dialfs) {
    char bitmap_path[strlen(path_base_dialfs) + strlen("/bitmap.dat") + 1];
    sprintf(bitmap_path, "%s/bitmap.dat", path_base_dialfs);

    FILE* file = fopen(bitmap_path, "wb");
    if (!file) {
        perror("Error abriendo bitmap.dat");
        exit(EXIT_FAILURE);
    }

    fwrite(bitmap, (block_count + 7) / 8, 1, file);

    fclose(file);

    printf("Bitmap guardado exitosamente en %s.\n", bitmap_path);
}

int encontrar_bloque_libre() {
    for (int i = 0; i < block_count; i++) {
        if (!(bitmap[i / 8] & (1 << (i % 8)))) {
            return i;
        }
    }
    return -1;
}

void marcar_bloque(int bloque, int ocupado) {
    if (ocupado) {
        bitmap[bloque / 8] |= (1 << (bloque % 8));
    } else {
        bitmap[bloque / 8] &= ~(1 << (bloque % 8));
    }
}
 //BITMAP

// METADATA
void crear_metadata(char* nombre_archivo, int bloque_inicial, int tamano_archivo) {
    FILE* file = fopen(nombre_archivo, "w");
    if (!file) {
        perror("Error creando archivo de metadata");
        exit(1);
    }
    fprintf(file, "BLOQUE_INICIAL=%d\nTAMANIO_ARCHIVO=%d\n", bloque_inicial, tamano_archivo);
    fclose(file);
}

void leer_metadata(char* nombre_archivo, int* bloque_inicial, int* tamano_archivo) {
    FILE* file = fopen(nombre_archivo, "r");
    if (!file) {
        perror("Error leyendo archivo de metadata");
        exit(1);
    }
    fscanf(file, "BLOQUE_INICIAL=%d\nTAMANIO_ARCHIVO=%d\n", bloque_inicial, tamano_archivo);
    fclose(file);
}

// METADATA
void compactar_fs() {
    int bloque_libre_actual = 0;
    for (int i = 0; i < block_count; i++) {
        if (!(bitmap[i / 8] & (1 << (i % 8)))) {
            int bloque_actual = i;
            // Buscar la próxima secuencia de bloques libres
            while (bloque_actual < block_count && !(bitmap[bloque_actual / 8] & (1 << (bloque_actual % 8)))) {
                bloque_actual++;
            }

            if (bloque_actual < block_count) {
                for (int j = bloque_libre_actual; j < bloque_actual; j++) {
                    marcar_bloque(j, 0); // Marco bloques libres en el bitmap

                }
                for (int j = i; j < bloque_actual; j++) {
                    marcar_bloque(j, 1);  // Marco nuevos bloques como ocupados en el bitmap

                }
                bloque_libre_actual = bloque_actual;
            }
        }
    }
}
//ARCHIVOS
void crear_archivo_en_dialfs(char* nombre_archivo) {
    int bloque_libre = encontrar_bloque_libre();

    if (bloque_libre != -1) {
        marcar_bloque(bloque_libre, 1);
        crear_metadata(nombre_archivo, bloque_libre, 0); // Archivo creado con tamaño 0
        //VER
        t_paquete* paquete = crear_paquete(PONERCODIGOOP);
        agregar_string_a_paquete(paquete, nombre_archivo);
        enviar_paquete(paquete, conexion_memoria);
        eliminar_paquete(paquete);
        //VER
    } else {
        printf("No hay bloques libres disponibles.\n");
    }
}

void eliminar_archivo_en_dialfs(char* nombre_archivo) {

    int bloque_inicial, tamano_archivo;
    leer_metadata(nombre_archivo, &bloque_inicial, &tamano_archivo);
    int bloques_necesarios = (tamano_archivo + block_size - 1) / block_size;

    for (int i = 0; i < bloques_necesarios; i++) {
        marcar_bloque(bloque_inicial + i, 0); // Libera los bloques
    }

    remove(nombre_archivo); // Elimina el archivo de metadata

    //VER BORRAR ARCHIVO EN MEMORIA
    
    //VER
}

void truncar_archivo_en_dialfs(const char* nombre_archivo, uint32_t nuevo_tamano) {
    int bloque_inicial, tamano_actual;
    leer_metadata(nombre_archivo, &bloque_inicial, &tamano_actual);
    int bloques_actuales = (tamano_actual + block_size - 1) / block_size;
    int bloques_nuevos = (nuevo_tamano + block_size - 1) / block_size;

    if (bloques_nuevos > bloques_actuales) {
        // Verificar si se necesita compactar
        if (!hay_espacio_contiguo(bloques_nuevos - bloques_actuales)) {
            compactar_fs();
            sleep(retraso_compactacion);
        }
        // Asignar nuevos bloques
        for (int i = bloques_actuales; i < bloques_nuevos; i++) {
            marcar_bloque(bloque_inicial + i, 1);
        }
    } else if (bloques_nuevos < bloques_actuales) {
        // Liberar bloques sobrantes
        for (int i = bloques_nuevos; i < bloques_actuales; i++) {
            marcar_bloque(bloque_inicial + i, 0);
        }
    }

    actualizar_metadata(nombre_archivo, bloque_inicial, nuevo_tamano);
}
void escribir_en_archivo_dialfs(char* nombre_archivo, uint32_t direccion_logica, uint32_t tamano, uint32_t puntero_archivo) {
    int bloque_inicial, tamano_archivo;
    leer_metadata(nombre_archivo, &bloque_inicial, &tamano_archivo);

    void* datos = contenido_obtenido_de_memoria(direccion_logica, tamano, 0);

    FILE* archivo = fopen(nombre_archivo, "r+b");
    if (archivo == NULL) {
        printf("Error al abrir el archivo.\n");
        free(datos);
        return;
    }

    fseek(archivo, puntero_archivo, SEEK_SET);
    fwrite(datos, 1, tamano, archivo);
    fclose(archivo);

    free(datos);

    if (puntero_archivo + tamano > tamano_archivo) {
        actualizar_metadata(nombre_archivo, bloque_inicial, puntero_archivo + tamano);
    }
}

void leer_desde_archivo_dialfs(char* nombre_archivo, uint32_t direccion_logica, uint32_t tamano, uint32_t puntero_archivo) {
    FILE* archivo = fopen(nombre_archivo, "rb");
    if (archivo == NULL) {
        printf("Error al abrir el archivo.\n");
        return;
    }

    fseek(archivo, puntero_archivo, SEEK_SET);
    void* datos = malloc(tamano);
    fread(datos, 1, tamano, archivo);
    fclose(archivo);

    enviar_a_memoria_para_escribir(direccion_logica, datos, tamano, 0);

    free(datos);
}
//ARCHIVOS
void crear_archivo_de_bloques(char* path_base_dialfs) {
    off_t file_size = (off_t)block_count * block_size;

    char bloques_path[strlen(path_base_dialfs) + strlen("/bloques.dat") + 1];
    sprintf(bloques_path, "%s/bloques.dat", path_base_dialfs);

    // Abrir el archivo para escritura
    FILE* file = fopen(bloques_path, "wb");
    if (!file) {
        perror("Error abriendo bloques.dat");
        exit(EXIT_FAILURE);
    }

    // Escribir datos de prueba (opcional)
    char buffer[block_size]; // Buffer para escribir bloques
    memset(buffer, 0, block_size); // Inicializar a 0

    for (int i = 0; i < block_count; ++i) {
        fwrite(buffer, block_size, 1, file);
    }

    // Cerrar el archivo
    fclose(file);

    printf("Archivo bloques.dat creado exitosamente en %s.\n", bloques_path);
}

void iniciarInterfazDialFS(int socket, t_config* config, char* nombre){
    int tiempo_pausa = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");
    block_size = config_get_int_value(config, "BLOCK_SIZE");
    block_count = config_get_int_value(config, "BLOCK_COUNT");
    retraso_compactacion = config_get_int_value(config, "RETRASO_COMPACTACION");
    char* path_base_dialfs = config_get_string_value(config, "PATH_BASE_DIALFS");
    crear_archivo_de_bloques(path_base_dialfs);
    cargar_bitmap(path_base_dialfs);

    t_paquete* paquete = crear_paquete(CONEXION_DIAL_FS);
    enviar_paquete(paquete, conexion_memoria);
    eliminar_paquete(paquete);

    while (1) {
        ssize_t reciv = recibir_operacion(socket);

        if (reciv < 0) {
            exit(-1);
        }

        switch (reciv) {
            case IO_FS_CREATE:
                char* nombre_archivo = recibir_string(socket);
                crear_archivo_en_dialfs(nombre_archivo);
                free(nombre_archivo);
                break;

            case IO_FS_DELETE:
                char* nombre_archivo = recibir_string(socket);
                eliminar_archivo_en_dialfs(nombre_archivo);
                free(nombre_archivo);
                break;

            case IO_FS_TRUNCATE:
                char* nombre_archivo = recibir_string(socket);
                uint32_t nuevo_tamano = recibir_tamano(socket); // Función para recibir el tamaño del registro
                truncar_archivo_en_dialfs(nombre_archivo, nuevo_tamano);
                free(nombre_archivo);
                break;

            case IO_FS_WRITE:
                char* nombre_archivo = recibir_string(socket);
                uint32_t direccion_logica = recibir_direccion(socket);
                uint32_t tamano = recibir_tamano(socket);
                uint32_t puntero_archivo = recibir_puntero(socket);
                escribir_en_archivo_dialfs(nombre_archivo, direccion_logica, tamano, puntero_archivo);
                free(nombre_archivo);
                break;

            case IO_FS_READ:
                char* nombre_archivo = recibir_string(socket);
                uint32_t direccion_logica = recibir_direccion(socket);
                uint32_t tamano = recibir_tamano(socket);
                uint32_t puntero_archivo = recibir_puntero(socket);
                leer_desde_archivo_dialfs(nombre_archivo, direccion_logica, tamano, puntero_archivo);
                free(nombre_archivo);
                break;

            default:
                break;
        }

        sleep(tiempo_pausa);

        t_paquete* paquete = crear_paquete(OPERACION_FINALIZADA);
        enviar_paquete(paquete ,socket);
        eliminar_paquete(paquete);
    }
    guardar_bitmap("bitmap.dat");
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

    free(bitmap);
    return 0;
}