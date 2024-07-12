#include <stdlib.h>
#include <stdio.h>
#include <readline/readline.h>
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
        usleep(tiempo_pausa * unidades_trabajo);

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
    
    t_paquete* paquete = crear_paquete(CONEXION_STDIN);
    agregar_string_a_paquete(paquete, nombre);
    enviar_paquete(paquete ,socket);
    eliminar_paquete(paquete);

    paquete = crear_paquete(CONEXION_STDIN);
    enviar_paquete(paquete, conexion_memoria);
    eliminar_paquete(paquete);

    while(1) {
        ssize_t reciv = recibir_operacion(socket);
        
        if (reciv < 0) {
            exit(-1);
        }

        char* texto = readline(">");

        int enviado = 0;
        t_buffer* buffer = recibir_buffer(socket);
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
void crear_metadata(char* path, char* nombre_archivo, int bloque_inicial, int tamano_archivo) {
    size_t len = strlen(path) + strlen(nombre_archivo) + 2; // 1 para '/' y 1 para '\0'
    char* ruta_completa = (char*)malloc(len);

    if (!ruta_completa) {
        perror("Error al asignar memoria para la ruta completa");
        exit(1);
    }

    snprintf(ruta_completa, len, "%s/%s", path, nombre_archivo);

    FILE* file = fopen(ruta_completa, "w");
    if (!file) {
        perror("Error creando archivo de metadata");
        free(ruta_completa);
        exit(1);
    }
    fprintf(file, "BLOQUE_INICIAL=%d\nTAMANIO_ARCHIVO=%d\n", bloque_inicial, tamano_archivo);
    fclose(file);

    free(ruta_completa);
}

void leer_metadata(char* path, char* nombre_archivo, int* bloque_inicial, int* tamano_archivo) {
   size_t len = strlen(path) + strlen(nombre_archivo) + 2;
    char* ruta_completa = (char*)malloc(len);

    if (!ruta_completa) {
        perror("Error al asignar memoria para la ruta completa");
        exit(1);
    }

    snprintf(ruta_completa, len, "%s/%s", path, nombre_archivo);

    FILE* file = fopen(ruta_completa, "r");
    if (!file) {
        perror("Error leyendo archivo de metadata");
        free(ruta_completa);
        exit(1);
    }

    fscanf(file, "BLOQUE_INICIAL=%d\nTAMANIO_ARCHIVO=%d\n", bloque_inicial, tamano_archivo);
    fclose(file);
    free(ruta_completa);
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
void crear_archivo_en_dialfs(char* path, char* nombre_archivo, int tam) {
    int bloque_libre = encontrar_bloque_libre();

    if (bloque_libre != -1) {
        marcar_bloque(bloque_libre, 1);
        crear_metadata(path,nombre_archivo, bloque_libre, tam); // Archivo creado con tamaño 0
    } else {
        printf("No hay bloques libres disponibles.\n");
    }
}

void eliminar_archivo_en_dialfs(char* path,char* nombre_archivo) {

    size_t len = strlen(path) + strlen(nombre_archivo) + 2; // 1 para '/' y 1 para '\0'
    char* ruta_completa = (char*)malloc(len);

    if (!ruta_completa) {
        perror("Error al asignar memoria para la ruta completa");
        exit(1);
    }
    snprintf(ruta_completa, len, "%s/%s", path, nombre_archivo);
    int bloque_inicial, tamano_archivo;
    leer_metadata(path, ruta_completa, &bloque_inicial, &tamano_archivo);
    int bloques_necesarios = (tamano_archivo + block_size - 1) / block_size;

    for (int i = 0; i < bloques_necesarios; i++) {
        marcar_bloque(bloque_inicial + i, 0); // Libera los bloques
    }

    if (remove(ruta_completa) != 0) {
        perror("Error eliminando archivo de metadata");
    }

    free(ruta_completa);
}

void truncar_archivo_en_dialfs(char*path ,char* nombre_archivo, int nuevo_tamano, int retraso_compactacion) {
    size_t len = strlen(path) + strlen(nombre_archivo) + 2;
    char* ruta_completa = (char*)malloc(len);

    if (!ruta_completa) {
        perror("Error al asignar memoria para la ruta completa");
        exit(1);
    }

    snprintf(ruta_completa, len, "%s/%s", path, nombre_archivo);

    int bloque_inicial, tamano_archivo;
    leer_metadata(path, nombre_archivo, &bloque_inicial, &tamano_archivo);
    int bloques_necesarios_nuevo = (nuevo_tamano + block_size - 1) / block_size;
    int bloques_necesarios_actual = (tamano_archivo + block_size - 1) / block_size;

    if (nuevo_tamano < tamano_archivo) {
        for (int i = bloques_necesarios_nuevo; i < bloques_necesarios_actual; i++) {
            marcar_bloque(bloque_inicial + i, 0);
        }
    } else if (nuevo_tamano > tamano_archivo) {
        int espacio_contiguo = 1;
        for (int i = bloque_inicial + bloques_necesarios_actual; i < bloque_inicial + bloques_necesarios_nuevo; i++) {
            if (i >= block_count || (bitmap[i / 8] & (1 << (i % 8)))) {
                espacio_contiguo = 0;
                break;
            }
        }
        if (!espacio_contiguo) {
            compactar_fs();
            sleep(retraso_compactacion);
        }
        for (int i = bloques_necesarios_actual; i < bloques_necesarios_nuevo; i++) {
            marcar_bloque(bloque_inicial + i, 1);
        }
    }

    FILE* file = fopen(ruta_completa, "w");
    if (!file) {
        perror("Error actualizando archivo de metadata");
        free(ruta_completa);
        exit(1);
    }
    fprintf(file, "BLOQUE_INICIAL=%d\nTAMANIO_ARCHIVO=%d\n", bloque_inicial, nuevo_tamano);
    fclose(file);
    free(ruta_completa);
}
void escribir_en_archivo_dialfs(char* path, char* nombre_archivo, char* texto) {
    size_t len = strlen(path) + strlen(nombre_archivo) + 2;
    char* ruta_completa = (char*)malloc(len);

    if (!ruta_completa) {
        perror("Error al asignar memoria para la ruta completa");
        exit(1);
    }

    snprintf(ruta_completa, len, "%s/%s", path, nombre_archivo);

    FILE* file = fopen(ruta_completa, "a");
    if (!file) {
        perror("Error abriendo archivo para escribir");
        free(ruta_completa);
        exit(1);
    }

    fprintf(file, "%s", texto);

    fclose(file);
    free(ruta_completa);
}

void leer_desde_archivo_dialfs(char* path, char* nombre_archivo) {
    size_t len = strlen(path) + strlen(nombre_archivo) + 2;
    char* ruta_completa = (char*)malloc(len);

    if (!ruta_completa) {
        perror("Error al asignar memoria para la ruta completa");
        exit(1);
    }

    snprintf(ruta_completa, len, "%s/%s", path, nombre_archivo);

    FILE* file = fopen(ruta_completa, "r");
    if (!file) {
        perror("Error abriendo archivo para lectura");
        free(ruta_completa);
        exit(1);
    }

    char* buffer;//dinamico?
    while (fgets(buffer, sizeof(buffer), file)) {
        printf("%s", buffer);
    }

    fclose(file);
    free(ruta_completa);
}
//ARCHIVOS
void crear_archivo_de_bloques(char* path_base_dialfs,char* nombre, int tam) {
    crear_archivo_en_dialfs(path_base_dialfs, nombre, tam);
    printf("Archivo bloques.dat creado exitosamente en %s/%s.dat.\n", path_base_dialfs, nombre);
}

void iniciarInterfazDialFS(int socket, t_config* config, char* nombre){
    int tiempo_pausa = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");
    block_size = config_get_int_value(config, "BLOCK_SIZE");
    block_count = config_get_int_value(config, "BLOCK_COUNT");
    retraso_compactacion = config_get_int_value(config, "RETRASO_COMPACTACION");
    char* path_base_dialfs = config_get_string_value(config, "PATH_BASE_DIALFS");
    int tam_bloq_dat = block_size*block_count;

    cargar_bitmap(path_base_dialfs);

    crear_archivo_de_bloques(path_base_dialfs,"bloques.dat",tam_bloq_dat);

    t_paquete* paquete = crear_paquete(CONEXION_DIAL_FS);
    enviar_paquete(paquete, conexion_memoria);
    eliminar_paquete(paquete);

    while (1) {
        op_code reciv = recibir_operacion(socket);

        if (reciv < 0) {
            exit(-1);
        }
        t_buffer* buffer = recibir_buffer(socket);

                char* nombre_archivo = buffer_read_string(buffer);

        switch (reciv) {
            case ENVIAR_DIALFS_CREATE:
                int tam = buffer_read_int(buffer);
                crear_archivo_en_dialfs(path_base_dialfs,nombre_archivo,tam);
                free(nombre_archivo);
                break;

            case ENVIAR_DIALFS_DELETE:
                eliminar_archivo_en_dialfs(path_base_dialfs,nombre_archivo);
                free(nombre_archivo);
                break;

            case ENVIAR_DIALFS_TRUNCATE:
                int nuevo_tamano = buffer_read_int(buffer);
                truncar_archivo_en_dialfs(path_base_dialfs,nombre_archivo, nuevo_tamano, retraso_compactacion);
                free(nombre_archivo);
                break;

            case ENVIAR_DIALFS_WRITE:
                char* texto = buffer_read_string(buffer); //
                escribir_en_archivo_dialfs(path_base_dialfs, nombre_archivo, texto);
                free(nombre_archivo);
                free(texto);
                break;

            case ENVIAR_DIALFS_READ:
                leer_desde_archivo_dialfs(path_base_dialfs, nombre_archivo);
                free(nombre_archivo);
                break;
            default:
                break;
        }
        free(buffer);
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
        iniciarInterfazSTDIN(conexion_kernel, nuevo_config, "TECLADO");
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