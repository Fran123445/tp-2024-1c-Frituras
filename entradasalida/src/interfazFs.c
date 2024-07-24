#include "interfazFs.h"
#include "interfazStd.h"

t_bitarray* bitmap;
int block_count;
int block_size;
int retraso_compactacion;
char* path_base_dialfs;
FILE* bloques_dat;
char* archivos_metadata;
int pid;

void _printearBitarray() {
    for(int i = 0; i < bitarray_get_max_bit(bitmap); i++) {
        printf("%d", bitarray_test_bit(bitmap, i));
    }
    printf("\n");
}

char* cargar_lista_archivos() {
    char* listaArchivos = rutacompleta("archivosMetadata");
    FILE* file = fopen(listaArchivos, "r+");
    if(!file) {
        archivos_metadata = string_new();
        
    } else {
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        fseek(file, 0, SEEK_SET);
        
        archivos_metadata = malloc(file_size + 1);
        if (archivos_metadata == NULL) {
            perror("Error al asignar memoria para archivos_metadata");
            fclose(file);
            return NULL;
        }
        
        int cantidadLeido = fread(archivos_metadata, 1, file_size, file);

        if (cantidadLeido != 1) {
            log_error(logger, "No se pudo leer la lista de archivos");
        }

        archivos_metadata[file_size] = '\0'; // Chequear  que la cadena termine en null

        fclose(file);
    }
    
    free(listaArchivos);
    return archivos_metadata;
}

void guardar_lista_archivos() {
    char* listaArchivos = rutacompleta("archivosMetadata");
    FILE* file = fopen(listaArchivos, "w+");

    fwrite(archivos_metadata, strlen(archivos_metadata)+1, 1, file);

    free(listaArchivos);
    fclose(file);
}

 //BITMAP
t_bitarray* iniciar_bitmap_bloques(int cant_bloques){
    int tamanioBitmap = cant_bloques / 8;
    char* bitarray_memoria_usuario = calloc(tamanioBitmap, sizeof(char));

    if (!bitarray_memoria_usuario){
        fprintf(stderr, "Error al crear puntero al bitarray");
        exit(EXIT_FAILURE);
    }
    t_bitarray* mapa_de_bloques = bitarray_create_with_mode(bitarray_memoria_usuario, tamanioBitmap, LSB_FIRST); // se lee el bit - significativo primero
    if(mapa_de_bloques == NULL){
        free(bitarray_memoria_usuario);
        fprintf(stderr, "Error al crear el bitarray");
        exit(EXIT_FAILURE);
    }
    return mapa_de_bloques;
}

void abrir_bloques_dat(){
    char* path = rutacompleta("bloques.dat");

    bloques_dat = fopen(path, "rb+");
    if (!bloques_dat) {
        bloques_dat = fopen(path, "wb+");
        
        int resultado = ftruncate(fileno(bloques_dat), block_count*block_size);
        if (resultado != 0) {
            log_error(logger, "PID: %d - Error al truncar", pid);
        }
    }

    free(path);
}

void cargar_bitmap() {
    char* bitmap_path = rutacompleta("bitmap.dat");

    FILE* file = fopen(bitmap_path, "rb+");
    if (!file) {
        file = fopen(bitmap_path, "wb+");
        if (!file) {
            perror("Error creando bitmap.dat");
            exit(EXIT_FAILURE);
        }

        bitmap = iniciar_bitmap_bloques(block_count);
        if (!bitmap) {
            perror("Error de memoria");
            exit(EXIT_FAILURE);
        }
        //bitarray_set_bit(bitmap, 0);
        fclose(file);
    } else {
        // Si el archivo existe lo cargo en memoria
        bitmap = iniciar_bitmap_bloques(block_count);

        int cantidadLeida = fread(bitmap->bitarray, block_count/8, 1, file);

        if (cantidadLeida != 1) {
            log_error(logger, "Error al leer bitmap.dat");
        }

        fclose(file);
    }

    printf("Bitmap cargado exitosamente desde %s.\n", bitmap_path);
    free(bitmap_path);
}

char* rutacompleta(char* nombre_archivo){
    size_t len = strlen(path_base_dialfs) + strlen(nombre_archivo) + 2;
    char* ruta_completa = (char*)malloc(len);
    snprintf(ruta_completa, len, "%s/%s", path_base_dialfs, nombre_archivo);
    return ruta_completa;
}

void guardar_bitmap() {
    char* bitmap_path = rutacompleta("bitmap.dat");

    FILE* file = fopen(bitmap_path, "wb");
    if (!file) {
        perror("Error abriendo bitmap.dat");
        exit(EXIT_FAILURE);
    }

    fwrite(bitmap->bitarray, block_count / 8, 1, file);

    free(bitmap_path);
    fclose(file);
}

int encontrar_bloque_libre() {
    for (int i = 0; i < block_count; i++) {
        if (!bitarray_test_bit(bitmap, i)) {
            return i;
        }
    }
    return -1;
}

void marcar_bloque(int bloque, int ocupar) {
    if (bloque < 0 || bloque >= block_count) {
        fprintf(stderr, "Error: Bloque fuera de rango: %d\n", bloque);
        return;
    }
    if (ocupar) {
        bitarray_set_bit(bitmap, bloque);
    } else {
        bitarray_clean_bit(bitmap, bloque);
    }
}

// METADATA
void crear_metadata(char* nombre_archivo, int bloque_inicial, int tamano_archivo, bool modificar){
    char* ruta_completa = rutacompleta(nombre_archivo);
    if (!ruta_completa) {
        perror("Error al asignar memoria para la ruta completa");
        exit(1);
    }

    FILE* file = fopen(ruta_completa, "wb+");
    if (!file) {
        perror("Error creando archivo de metadata");
        free(ruta_completa);
        exit(1);
    }
    fprintf(file, "BLOQUE_INICIAL=%d\nTAMANIO_ARCHIVO=%d\n", bloque_inicial, tamano_archivo);
    fclose(file);

    if (!modificar) {
        string_append_with_format(&archivos_metadata, "%s ", nombre_archivo);
    }

    free(ruta_completa);
}

void leer_metadata(char* nombre_archivo, int* bloque_inicial, int* tamano_archivo){
    char* ruta_completa = rutacompleta(nombre_archivo);

    if (!ruta_completa) {
        perror("Error al asignar memoria para la ruta completa");
        exit(1);
    }

    FILE* file = fopen(ruta_completa, "rb+");
    if (!file) {
        perror("Error leyendo archivo de metadata");
        free(ruta_completa);
        exit(1);
    }

    int cantidadLeida = fscanf(file, "BLOQUE_INICIAL=%d\nTAMANIO_ARCHIVO=%d\n", bloque_inicial, tamano_archivo);

    if (cantidadLeida != 2) {
        log_error(logger, "PID: %d - Error al leer metadata de %s", pid, nombre_archivo);
    }

    fclose(file);
    free(ruta_completa);
}

void mover_bloque(int bloque_origen, int bloque_destino) {
    char buffer[block_size];

    // Leer datos del bloque de origen
    if (fseek(bloques_dat, bloque_origen * block_size, SEEK_SET) != 0) {
        perror("Error posicionando el puntero de archivo en el bloque de origen");
        return;
    }
    if (fread(buffer, block_size, 1, bloques_dat) != 1) {
        perror("Error leyendo datos del bloque de origen");
        return;
    }

    // Escribir datos en el bloque de destino
    if (fseek(bloques_dat, bloque_destino * block_size, SEEK_SET) != 0) {
        perror("Error posicionando el puntero de archivo en el bloque de destino");
        return;
    }
    if (fwrite(buffer, block_size, 1, bloques_dat) != 1) {
        perror("Error escribiendo datos en el bloque de destino");
        return;
    }

}

char* encontrar_archivo_por_bloque(int bloque) {
    char* archivos_metadata_copia = strdup(archivos_metadata); // Crear una copia para manipular
    char* token = strtok(archivos_metadata_copia," ");
    int tamanio_archivo_bloque;
    while (token != NULL) {
        int bloque_inicial, tamano_archivo;
        leer_metadata(token, &bloque_inicial, &tamano_archivo); //Consigo el bloque inicial y el tamaño

        tamanio_archivo_bloque = ceil(tamano_archivo/(float)block_size);

        if (bloque >= bloque_inicial && bloque <= bloque_inicial + tamanio_archivo_bloque) { //Chequeo si el bloque esta dentro del rango del archivo
            char* token_retorno = strdup(token);
            free(archivos_metadata_copia);
            return token_retorno; // Devolver una copia del nombre del archivo
        }

        token = strtok(NULL, " ");
    }

    free(archivos_metadata_copia);
    return NULL; // No se encontró un archivo que contenga el bloque
}

void mover_archivo(int bloque_libre_actual, int bloque_inicial, int tamanio_archivo) {
    for(int i = bloque_inicial; i < bloque_inicial + tamanio_archivo; i++) {
        // Mover el contenido del bloque i al bloque bloque_libre_actual
        mover_bloque(i, bloque_libre_actual);

        // Marcar el bloque i como libre y el bloque bloque_libre_actual como ocupado
        marcar_bloque(i, 0);
        marcar_bloque(bloque_libre_actual, 1);

        bloque_libre_actual++;
    }
}

// METADATA

void compactar_fs(){
    usleep(retraso_compactacion);

    int bloque_libre_actual = 0;

    log_info(logger, "PID: %d - Inicio Compactación.", pid);

    bloque_libre_actual = encontrar_bloque_libre();

    if (bloque_libre_actual >= block_count) {
        log_warning(logger, "No hay bloques libres para compactar.");
        log_info(logger, "PID: %d - Fin Compactación.", pid);
        return;
    }

    int tamanio_en_bloques = 0;

    for (int i = bloque_libre_actual + 1; i < block_count; i += bloque_libre_actual + 1) {
        if (bitarray_test_bit(bitmap, i)) {
            char* nombre_archivo = encontrar_archivo_por_bloque(i);
            if (!nombre_archivo) {
                log_error(logger, "PID: %d - No se encontró el archivo para el bloque %d.", pid, i);
                continue;
            }

            int bloque_inicial, tamano_archivo;
            leer_metadata(nombre_archivo, &bloque_inicial, &tamano_archivo);

            tamanio_en_bloques = ceil(tamano_archivo/(float)block_size);

            mover_archivo(bloque_libre_actual, bloque_inicial, tamanio_en_bloques);

            // Actualizar la metadata del archivo 
            crear_metadata(nombre_archivo, bloque_libre_actual, tamano_archivo, 1);

            free(nombre_archivo);

            _printearBitarray();
            // Encontrar el siguiente bloque libre
            while (bloque_libre_actual < block_count && bitarray_test_bit(bitmap, bloque_libre_actual)) {
                bloque_libre_actual++;
            }
        }
    }

    log_info(logger, "PID: %d - Fin Compactación.", pid);
}

//ARCHIVOS

void asignar_bloques(int bloque_inicial, int bloques_necesarios) {
    for(int i = bloque_inicial; i < bloque_inicial + bloques_necesarios; i++) {
        marcar_bloque(i, 1);
    }
}

void crear_archivo_en_dialfs(char* nombre_archivo, int tam){ // CAMBIARLO PARA QUE SOLO CONSIDERE UN BLOQUE
    int bloque_libre = encontrar_bloque_libre();

    if (bloque_libre == -1) {
        log_error(logger, "PID: %d - No hay bloques libres disponibles para crear el archivo", pid);
    }

    marcar_bloque(bloque_libre, 1);

    crear_metadata(nombre_archivo, bloque_libre, tam, 0); // Archivo creado con tamaño 0
}

void eliminar_archivo_de_lista(char* nombre_archivo) {
    char* archivo_a_reemplazar = string_new();
    string_append_with_format(&archivo_a_reemplazar, "%s ", nombre_archivo);

    char* temp = string_replace(archivos_metadata, archivo_a_reemplazar, "");

    free(archivos_metadata);
    free(archivo_a_reemplazar);
    archivos_metadata = temp;
    string_trim(&archivos_metadata);
}

void eliminar_archivo_en_dialfs(char* nombre_archivo){
    char* archivo = rutacompleta(nombre_archivo);
    int bloque_inicial, tamano_archivo;
    leer_metadata(nombre_archivo, &bloque_inicial, &tamano_archivo); //VER
    int bloques_necesarios = (tamano_archivo + block_size - 1) / block_size;

    for (int i = 0; i < bloques_necesarios; i++) {
        marcar_bloque(bloque_inicial + i, 0); // Libera los bloques
    }

    eliminar_archivo_de_lista(nombre_archivo);

    remove(archivo);
    free(archivo);
}

int existeEspacioContiguo(int bloquesNecesariosArchivo) {
    int cantidad_contiguos = 0;

    for (int i = 0; i < block_count; i++) {
        if (!bitarray_test_bit(bitmap, i)) {
            cantidad_contiguos++;
            if (cantidad_contiguos >= bloquesNecesariosArchivo) return i-bloquesNecesariosArchivo + 1; 
        } else {
            cantidad_contiguos = 0;
        }
    }

    return 0;
}

void truncar_archivo_en_dialfs(char* nombre_archivo, uint32_t nuevo_tamano, int retraso_compactacion) {
    int bloque_inicial, tamano_archivo;
    leer_metadata(nombre_archivo, &bloque_inicial, &tamano_archivo);
    int bloques_necesarios_nuevo = ceil((nuevo_tamano) / (float) block_size);
    int bloques_necesarios_actual = ceil((tamano_archivo) / (float) block_size);
    int espacio_contiguo;
    
    if (bloques_necesarios_actual == 0) {
        bloques_necesarios_actual = 1;
    }

    if (nuevo_tamano < tamano_archivo) {// Reducir el tamaño del archivo si el nuevo tamaño es menor al actual

        for (int i = bloques_necesarios_nuevo; i < bloques_necesarios_actual; i++) {
            marcar_bloque(bloque_inicial + i, 0);
        }

    } else if (nuevo_tamano > tamano_archivo) {//Aumento el tamaño del archivo si el nuevo tamaño es mayor al actual
        
        int espacio_extra_disponible = 1;
        for (int i = bloques_necesarios_actual; i < bloques_necesarios_nuevo; i++) { // Verificar si hay suficiente espacio en los bloques actuales
            if (bitarray_test_bit(bitmap, bloque_inicial + i)) {
                espacio_extra_disponible = 0;
                break;
            }

        }
        if (espacio_extra_disponible){// No es necesario mover el archivo, solo ajustar el tamaño
            
            asignar_bloques(bloque_inicial, bloques_necesarios_nuevo);
            crear_metadata(nombre_archivo, bloque_inicial, nuevo_tamano, 1);

        } else {
            espacio_contiguo = existeEspacioContiguo(bloques_necesarios_nuevo);

            if (!espacio_contiguo) { //Si no hay espacio contiguo Compacto
                compactar_fs();
                leer_metadata(nombre_archivo, &bloque_inicial, &tamano_archivo);
                espacio_contiguo = existeEspacioContiguo(bloques_necesarios_nuevo);

                if (espacio_contiguo) {
                    mover_archivo(espacio_contiguo, bloque_inicial, bloques_necesarios_actual);
                    asignar_bloques(espacio_contiguo, bloques_necesarios_nuevo);
                    crear_metadata(nombre_archivo, espacio_contiguo, nuevo_tamano, 1);
                } else {
                    log_error(logger, "PID: %d - No hay espacio contiguo disponible", pid);
                }

            } else {
                mover_archivo(espacio_contiguo, bloque_inicial, bloques_necesarios_actual);
                asignar_bloques(espacio_contiguo, bloques_necesarios_nuevo);
                crear_metadata(nombre_archivo, espacio_contiguo, nuevo_tamano, 1);
            }
        }
    }
}


void escribir_en_archivo_dialfs(char* nombre_archivo, uint32_t direccion, uint32_t tamanio, uint32_t ubicacionPuntero, int pid){
    int bloque_inicial, tamanio_archivo;

    leer_metadata(nombre_archivo, &bloque_inicial, &tamanio_archivo);

    if (ubicacionPuntero + tamanio > tamanio_archivo) {
        // log_error(muy mal)
        exit(-1);
    }

    pedir_contenido_memoria(direccion, tamanio, pid);

    void* datos_a_escribir = recibir_contenido_memoria_fs();

    fseek(bloques_dat, bloque_inicial*block_size + ubicacionPuntero, SEEK_SET);

    fwrite(datos_a_escribir, tamanio, 1, bloques_dat);

    free(datos_a_escribir);
}

void leer_desde_archivo_dialfs(char* nombre_archivo, uint32_t direccion, uint32_t tamanio, uint32_t ubicacionPuntero, int pid) {

    int bloque_inicial, tamanio_archivo;

    leer_metadata(nombre_archivo, &bloque_inicial, &tamanio_archivo);
    
    if (ubicacionPuntero + tamanio > tamanio_archivo) {
        // log_error(muy mal)
        exit(-1);
    }

    void* datos_a_leer = malloc(tamanio);

    fseek(bloques_dat, bloque_inicial*block_size + ubicacionPuntero, SEEK_SET);

    int cantidadLeida = fread(datos_a_leer, tamanio, 1, bloques_dat);

    if (cantidadLeida != 1) {
        log_error(logger, "PID: %d - Error al leer %s", pid, nombre_archivo);
    }

    enviar_a_memoria_para_escribir(direccion, datos_a_leer, tamanio, pid);

    free(datos_a_leer);
}
//ARCHIVOS


void iniciarInterfazDialFS(t_config* config, char* nombre){
    int tiempo_pausa = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");
    block_size = config_get_int_value(config, "BLOCK_SIZE");
    block_count = config_get_int_value(config, "BLOCK_COUNT");
    retraso_compactacion = config_get_int_value(config, "RETRASO_COMPACTACION");
    path_base_dialfs = config_get_string_value(config, "PATH_BASE_DIALFS");

   t_paquete* paquete = crear_paquete(CONEXION_DIAL_FS);
    agregar_string_a_paquete(paquete, nombre);
    enviar_paquete(paquete, conexion_kernel);
    eliminar_paquete(paquete);

    paquete = crear_paquete(CONEXION_DIAL_FS);
    enviar_paquete(paquete, conexion_memoria);
    eliminar_paquete(paquete);

    cargar_lista_archivos();

    abrir_bloques_dat();

    cargar_bitmap();

    while (1) {
        op_code reciv = recibir_operacion(conexion_kernel);

        if (reciv <= 0) {
            break;
        }

        t_buffer* buffer = recibir_buffer(conexion_kernel);

        pid = buffer_read_int(buffer);
        char* nombre_archivo = buffer_read_string(buffer);



        switch (reciv) {
            case ENVIAR_DIALFS_CREATE:
                crear_archivo_en_dialfs(nombre_archivo,0);
                log_info(logger, "PID: %d - Crear Archivo: %s", pid, nombre_archivo);
                break;
            case ENVIAR_DIALFS_DELETE:
                eliminar_archivo_en_dialfs(nombre_archivo);
                log_info(logger, "PID: %d - Eliminar Archivo: %s", pid, nombre_archivo);
                break;

            case ENVIAR_DIALFS_TRUNCATE:
                uint32_t nuevo_tamano = buffer_read_uint32(buffer);
                truncar_archivo_en_dialfs(nombre_archivo, nuevo_tamano, retraso_compactacion);
                log_info(logger, "PID: %d - Truncar Archivo: %s - Tamaño: %d", pid, nombre_archivo, nuevo_tamano);
                break;

            case ENVIAR_DIALFS_WRITE:
                while (buffer->size > 0) {
                    uint32_t direccion = buffer_read_uint32(buffer);
                    uint32_t tamanio = buffer_read_uint32(buffer);
                    uint32_t ubicacionPuntero = buffer_read_uint32(buffer);
                    escribir_en_archivo_dialfs(nombre_archivo, direccion, tamanio, ubicacionPuntero, pid);
                    log_info(logger, "PID: %d - Escribir Archivo: %s - Tamaño a Escribir: %d - Puntero Archivo: %d", pid, nombre_archivo, tamanio, ubicacionPuntero);
                }
                break;
            case ENVIAR_DIALFS_READ:
                while (buffer->size > 0) {
                    uint32_t direccion = buffer_read_uint32(buffer);
                    uint32_t tamanio = buffer_read_uint32(buffer);
                    uint32_t ubicacionPuntero = buffer_read_uint32(buffer);
                    leer_desde_archivo_dialfs(nombre_archivo, direccion, tamanio, ubicacionPuntero, pid);      
                    log_info(logger, "PID: %d - Leer Archivo: %s - Tamaño a Leer: %d - Puntero Archivo: %d", pid, nombre_archivo, tamanio, ubicacionPuntero);
                }
                break;
            default:
                break;
        }
        _printearBitarray();
        liberar_buffer(buffer);
        free(nombre_archivo);
        usleep(tiempo_pausa);

        guardar_bitmap();

        guardar_lista_archivos();
    
        t_paquete* paquete = crear_paquete(OPERACION_FINALIZADA);
        enviar_paquete(paquete ,conexion_kernel);
        eliminar_paquete(paquete);
    } 
    free(bitmap->bitarray);
    bitarray_destroy(bitmap);
    fclose(bloques_dat);
    free(archivos_metadata);
}