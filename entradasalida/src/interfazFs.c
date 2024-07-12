#include "interfazFs.h"

t_bitarray* bitmap;
int block_count;
int block_size;
int retraso_compactacion;
char* path_base_dialfs;
FILE* bloques_dat;


 //BITMAP
t_bitarray* iniciar_bitmap_bloques(int cant_bloques){
    char* bitarray_memoria_usuario = calloc(cant_bloques/8, sizeof(char));
    if (!bitarray_memoria_usuario){
        fprintf(stderr, "Error al crear puntero al bitarray");
        exit(EXIT_FAILURE);
    }
    t_bitarray* mapa_de_bloques = bitarray_create_with_mode(bitarray_memoria_usuario, cant_bloques, LSB_FIRST); // se lee el bit - significativo primero
    if(mapa_de_bloques == NULL){
        free(bitarray_memoria_usuario);
        fprintf(stderr, "Error al crear el bitarray");
        exit(EXIT_FAILURE);
    }
    return mapa_de_bloques;
}

void abrir_bloques_dat(){
    char path[strlen(path_base_dialfs) + strlen("/bloques.dat") + 1];
    sprintf(path, "%s/bitmap.dat", path_base_dialfs);

    bloques_dat = fopen(path, "rb+");
    if (!file) {
        perror("Error abriendo bloques.dat");
        return;
    }

}

void cargar_bitmap() {
    char bitmap_path[strlen(path_base_dialfs) + strlen("/bitmap.dat") + 1];
    sprintf(bitmap_path, "%s/bitmap.dat", path_base_dialfs);

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
        bitarray_set_bit(bitmap, 0);
        fclose(file);
    } else {
        // Si el archivo existe lo cargo en memoria
        fseek(file, 0, SEEK_END);
        long size = ftell(file);
        fseek(file, 0, SEEK_SET);

        bitmap = iniciar_bitmap_bloques(block_count);

        if (!bitmap) {
            perror("Error de memoria");
            exit(EXIT_FAILURE);
        }

        fread(bitmap, size, 1, file);
        fclose(file);
    }

    printf("Bitmap cargado exitosamente desde %s.\n", bitmap_path);
}

char* rutacompleta(char* nombre_archivo){
    size_t len = strlen(path_base_dialfs) + strlen(nombre_archivo) + 2;
    char* ruta_completa = (char*)malloc(len);
    snprintf(ruta_completa, len, "%s/%s", path_base_dialfs, nombre_archivo);
    return ruta_completa;
}

void guardar_bitmap() {
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
        if (!bitarray_test_bit(bitmap, i)) {
            return i;
        }
    }
    return -1;
}

void marcar_bloque(int bloque, int ocupado){
    if (ocupado) {
        bitarray_clean_bit(bitmap, bloque);
    } else {
        bitarray_set_bit(bitmap, bloque);
    }
}
 //BITMAP

// METADATA
void crear_metadata(char* nombre_archivo, int bloque_inicial, int tamano_archivo){
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

    fscanf(file, "BLOQUE_INICIAL=%d\nTAMANIO_ARCHIVO=%d\n", bloque_inicial, tamano_archivo);
    fclose(file);
    free(ruta_completa);
}
/*
void mover_bloque(int bloque_origen, int bloque_destino) {
    char buffer[block_size];
    
    // Leer datos del bloque de origen
    if (fseek(file, bloque_origen * block_size, SEEK_SET) != 0) {
        perror("Error posicionando el puntero de archivo en el bloque de origen");
        fclose(file);
        return;
    }
    if (fread(buffer, block_size, 1, file) != 1) {
        perror("Error leyendo datos del bloque de origen");
        fclose(file);
        return;
    }

    // Escribir datos en el bloque de destino
    if (fseek(file, bloque_destino * block_size, SEEK_SET) != 0) {
        perror("Error posicionando el puntero de archivo en el bloque de destino");
        fclose(file);
        return;
    }
    if (fwrite(buffer, block_size, 1, file) != 1) {
        perror("Error escribiendo datos en el bloque de destino");
        fclose(file);
        return;
    }

    fclose(file);
}
*/
// METADATA
void compactar_fs(){
    int bloque_libre_actual = 0;
    
    while (bloque_libre_actual < block_count && bitarray_test_bit(bitmap, bloque_libre_actual)) {
        bloque_libre_actual++;
    }

    if (bloque_libre_actual >= block_count) {
        printf("No hay bloques libres para compactar.\n");
        return;
    }

    for (int i = bloque_libre_actual + 1; i < block_count; i++) {
        if (bitarray_test_bit(bitmap, i)) {
            char* nombre_archivo = encontrar_archivo_por_bloque(i);
            if (!nombre_archivo) {
                fprintf(stderr, "Error: no se encontró el archivo para el bloque %d\n", i);
                continue;
            }

            int bloque_inicial, tamano_archivo;
            leer_metadata(nombre_archivo, &bloque_inicial, &tamano_archivo);

            // Calcular el desplazamiento del bloque dentro del archivo
            int desplazamiento_bloque = i - bloque_inicial;

            // Mover el contenido del bloque i al bloque bloque_libre_actual
            mover_bloque(i, bloque_libre_actual);

            // Marcar el bloque i como libre y el bloque bloque_libre_actual como ocupado
            marcar_bloque(i, 0);
            marcar_bloque(bloque_libre_actual, 1);

            // Actualizar la metadata del archivo
            crear_metadata(nombre_archivo, bloque_inicial + (bloque_libre_actual - i), tamano_archivo);

            free(nombre_archivo);

            // Encontrar el siguiente bloque libre
            while (bloque_libre_actual < block_count && bitarray_test_bit(bitmap, bloque_libre_actual)) {
                bloque_libre_actual++;
            }
        }
    }

    printf("Compactación de FS completada.\n");
}
//ARCHIVOS
void crear_archivo_en_dialfs(char* nombre_archivo, int tam){
    int bloque_libre = encontrar_bloque_libre();

    if (bloque_libre != -1) {
        marcar_bloque(bloque_libre, 1);
        crear_metadata(nombre_archivo, bloque_libre, tam); // Archivo creado con tamaño 0
    } else {
        printf("No hay bloques libres disponibles.\n");
    }
}



void eliminar_archivo_en_dialfs(char* nombre_archivo){
    int bloque_inicial, tamano_archivo;
    leer_metadata(nombre_archivo, &bloque_inicial, &tamano_archivo); //VER
    int bloques_necesarios = (tamano_archivo + block_size - 1) / block_size;

    for (int i = 0; i < bloques_necesarios; i++) {
        marcar_bloque(bloque_inicial + i, 0); // Libera los bloques
    }

}

void truncar_archivo_en_dialfs(char* nombre_archivo, int nuevo_tamano, int retraso_compactacion){
    char* ruta_completa = rutacompleta(nombre_archivo);
    int bloque_inicial, tamano_archivo;
    leer_metadata(nombre_archivo, &bloque_inicial, &tamano_archivo);
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

    FILE* file = fopen(ruta_completa, "wb+");
    if (!file) {
        perror("Error actualizando archivo de metadata");
        exit(1);
    }
    fprintf(file, "BLOQUE_INICIAL=%d\nTAMANIO_ARCHIVO=%d\n", bloque_inicial, nuevo_tamano);
    fclose(file);
}
void escribir_en_archivo_dialfs(char* nombre_archivo, char* texto){
    char* ruta_completa = rutacompleta(nombre_archivo);

    if (!ruta_completa) {
        perror("Error al asignar memoria para la ruta completa");
        exit(1);
    }

    FILE* file = fopen(ruta_completa, "a+");
    if (!file) {
        perror("Error abriendo archivo para escribir");
        free(ruta_completa);
        exit(1);
    }

    fprintf(file, "%s", texto);

    fclose(file);
    free(ruta_completa);
}

void leer_desde_archivo_dialfs(char* nombre_archivo){
    char* ruta_completa = rutacompleta(nombre_archivo);

    if (!ruta_completa) {
        perror("Error al asignar memoria para la ruta completa");
        exit(1);
    }

    FILE* file = fopen(ruta_completa, "rb+");
    if (!file) {
        perror("Error abriendo archivo para lectura");
        free(ruta_completa);
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = malloc(file_size + 1);
    if (!buffer) {
        perror("Error al asignar memoria para el buffer");
        fclose(file);
        free(ruta_completa);
        exit(1);
    }

    size_t read_size = fread(buffer, 1, file_size, file);
    if (read_size != file_size) {
        perror("Error leyendo el archivo");
        free(buffer);
        fclose(file);
        free(ruta_completa);
        exit(1);
    }
    buffer[file_size] = '\0'; // Asegura que el buffer es null para imprimirlo como una cadena

    printf("%s", buffer);

    free(buffer);
    fclose(file);
    free(ruta_completa);
}
//ARCHIVOS
void crear_archivo_de_bloques(char* nombre, int tam){
    crear_archivo_en_dialfs(path_base_dialfs, nombre, tam);
    printf("Archivo bloques.dat creado exitosamente en %s/%s.dat.\n", path_base_dialfs, nombre);
}

void iniciarInterfazDialFS(t_config* config, char* nombre){
    int tiempo_pausa = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");
    block_size = config_get_int_value(config, "BLOCK_SIZE");
    block_count = config_get_int_value(config, "BLOCK_COUNT");
    retraso_compactacion = config_get_int_value(config, "RETRASO_COMPACTACION");
    path_base_dialfs = config_get_string_value(config, "PATH_BASE_DIALFS");
    int tam_bloq_dat = block_size*block_count;

    cargar_bitmap(path_base_dialfs);

    crear_archivo_de_bloques(path_base_dialfs,"bloques.dat",tam_bloq_dat);

    t_paquete* paquete = crear_paquete(CONEXION_DIAL_FS);
    enviar_paquete(paquete, conexion_memoria);
    eliminar_paquete(paquete);

    while (1) {
        op_code reciv = recibir_operacion(conexion_kernel);

        if (reciv < 0) {
            exit(-1);
        }
        t_buffer* buffer = recibir_buffer(conexion_kernel);

                char* nombre_archivo = buffer_read_string(buffer);

        switch (reciv) {
            case ENVIAR_DIALFS_CREATE:
                int tam = buffer_read_int(buffer);
                crear_archivo_en_dialfs(nombre_archivo,tam);
                free(nombre_archivo);
                break;

            case ENVIAR_DIALFS_DELETE:
                eliminar_archivo_en_dialfs(nombre_archivo);
                free(nombre_archivo);
                break;

            case ENVIAR_DIALFS_TRUNCATE:
                int nuevo_tamano = buffer_read_int(buffer);
                truncar_archivo_en_dialfs(nombre_archivo, nuevo_tamano, retraso_compactacion);
                free(nombre_archivo);
                break;

            case ENVIAR_DIALFS_WRITE:
                char* texto = buffer_read_string(buffer); //
                escribir_en_archivo_dialfs(nombre_archivo, texto);
                free(nombre_archivo);
                free(texto);
                break;

            case ENVIAR_DIALFS_READ:
                leer_desde_archivo_dialfs(nombre_archivo);
                free(nombre_archivo);
                break;
            default:
                break;
        }
        free(buffer);
        sleep(tiempo_pausa);

        t_paquete* paquete = crear_paquete(OPERACION_FINALIZADA);
        enviar_paquete(paquete ,conexion_kernel);
        eliminar_paquete(paquete);
    }
    guardar_bitmap("bitmap.dat");
}