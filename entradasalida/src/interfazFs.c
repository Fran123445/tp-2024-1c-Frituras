#include "interfazFs.h"

t_bitarray* bitmap;
int block_count;
int block_size;
int retraso_compactacion;
char* path_base_dialfs;
FILE* bloques_dat;
char* archivos_metadata;

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
        
        fread(archivos_metadata, 1, file_size, file);
        archivos_metadata[file_size] = '\0'; // Chequear  que la cadena termine en null

        fclose(file);
    }
    
    free(listaArchivos);
    return archivos_metadata;
}

void guardar_lista_archivos() {
    char* listaArchivos = rutacompleta("archivosMetadata");
    FILE* file = fopen(listaArchivos, "w+");

    fwrite(listaArchivos, strlen(listaArchivos), 1, file);
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
        ftruncate(fileno(bloques_dat), block_count*block_size);
        return;
    }

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
        fseek(file, 0, SEEK_END);
        long size = ftell(file);
        fseek(file, 0, SEEK_SET);

        bitmap = iniciar_bitmap_bloques(block_count);

        size_t bytes_leidos = fread(bitmap->bitarray, block_count/8, 1, file);

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
    char* bitmap_path = rutacompleta("bitmap.dat");

    FILE* file = fopen(bitmap_path, "wb");
    if (!file) {
        perror("Error abriendo bitmap.dat");
        exit(EXIT_FAILURE);
    }

    size_t bytes_escritos = fwrite(bitmap->bitarray, block_count / 8, 1, file);

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

void marcar_bloque(int bloque, int ocupar){
    if (ocupar) {
        bitarray_set_bit(bitmap, bloque);
    } else {
        bitarray_clean_bit(bitmap, bloque);
    }
}
 //BITMAP

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
        string_append_with_format(&archivos_metadata, " %s", nombre_archivo);
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

    fscanf(file, "BLOQUE_INICIAL=%d\nTAMANIO_ARCHIVO=%d\n", bloque_inicial, tamano_archivo);
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
    char* token = strtok(archivos_metadata_copia, " ");
    while (token != NULL) {
        int bloque_inicial, tamano_archivo;
        leer_metadata(token, &bloque_inicial, &tamano_archivo); //Consigo el bloque inicial y el tamaño

        if (bloque >= bloque_inicial && bloque < bloque_inicial + tamano_archivo) { //Chequeo si el bloque esta dentro del rango del archivo
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

            mover_archivo(bloque_libre_actual, bloque_inicial, tamano_archivo);

            // Actualizar la metadata del archivo
            crear_metadata(nombre_archivo, bloque_inicial + (bloque_libre_actual - i), tamano_archivo, 1);

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

bool chequearBloquesContiguosDisponibles(int bloques_necesarios, int bloque_inicial) {
    for(int i = bloque_inicial+1; i < bloque_inicial+bloques_necesarios; i++) {
        if (bitarray_test_bit(bitmap, i)) return false;
    }

    return true;
}

bool verificarBloquesContiguos(int bloques_necesarios, int bloque_inicial) {
    bool bloques_disp = chequearBloquesContiguosDisponibles(bloques_necesarios, bloque_inicial);

    if (!bloques_disp) {
        //compactar()
        return chequearBloquesContiguosDisponibles(bloques_necesarios, bloque_inicial);   
    }

    return true;
}

bool asignar_bloques(int tam, int bloque_inicial) {

    int bloques_necesarios = ceil(tam/block_size);

    if (!verificarBloquesContiguos(bloques_necesarios, bloque_inicial)) {
        //log_error(log, "No hay bloques libres disponibles");
        return false;
    }

    for(int i = bloque_inicial; i < bloque_inicial + bloques_necesarios; i++) {
        marcar_bloque(i, 1);
    }

    return true;
}

void crear_archivo_en_dialfs(char* nombre_archivo, int tam){ // CAMBIARLO PARA QUE SOLO CONSIDERE UN BLOQUE
    int bloque_libre = encontrar_bloque_libre();

    if (!asignar_bloques(tam, bloque_libre)) {
        return;
    }

    crear_metadata(nombre_archivo, bloque_libre, tam, 0); // Archivo creado con tamaño 0
}

void eliminar_archivo_de_lista(char* nombre_archivo) {
    char* archivo_a_reemplazar = string_new();
    string_append_with_format(&archivo_a_reemplazar, " %s ", nombre_archivo);

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
}


void truncar_archivo_en_dialfs(char* nombre_archivo, int nuevo_tamano, int retraso_compactacion){
    char* ruta_completa = rutacompleta(nombre_archivo);
    int bloque_inicial, tamano_archivo;
    leer_metadata(nombre_archivo, &bloque_inicial, &tamano_archivo);
    int bloques_necesarios_nuevo = (nuevo_tamano + block_size - 1) / block_size;
    int bloques_necesarios_actual = (tamano_archivo + block_size - 1) / block_size;

    if (nuevo_tamano < tamano_archivo) {
        for (int i = bloques_necesarios_nuevo; i < bloques_necesarios_actual; i++) {
            marcar_bloque(bloque_inicial + i, 0); // Reducir el tamaño del archivo si el nuevo tamaño es menor al actual
        }
    } else if (nuevo_tamano > tamano_archivo) { //Aumento el tamaño del archivo si el nuevo tamaño es mayor al actual
        int espacio_contiguo = 1;
        for (int i = bloque_inicial + bloques_necesarios_actual; i < bloque_inicial + bloques_necesarios_nuevo; i++) { 
            if (i >= block_count || bitarray_test_bit(bitmap, i)) { //Compruebo si el espacio contiguo esta ocupado
                espacio_contiguo = 0;   
                break;
            }
        }
        if (!espacio_contiguo) { //Si no hay espacio contiguo Compacto
            compactar_fs();
            sleep(retraso_compactacion);
        }
        for (int i = bloques_necesarios_actual; i < bloques_necesarios_nuevo; i++) {
            marcar_bloque(bloque_inicial + i, 1);   //Marco los nuevos bloques como ocupados
        }
    }

    crear_metadata(nombre_archivo, bloque_inicial, nuevo_tamano, 1);
}


void escribir_en_archivo_dialfs(char* nombre_archivo, int direccion, int tamanio, int ubicacionPuntero, int pid){
    int bloque_inicial, tamanio_archivo;

    leer_metadata(nombre_archivo, &bloque_inicial, &tamanio_archivo);

    if (ubicacionPuntero + tamanio > tamanio_archivo) {
        // log_error(muy mal)
        exit(-1);
    }

    pedir_contenido_memoria((uint32_t)direccion, (uint32_t)tamanio, pid);

    void* datos_a_escribir = recibir_contenido_memoria();

    fseek(bloques_dat, bloque_inicial*block_size + ubicacionPuntero, SEEK_SET);

    fwrite(datos_a_escribir, tamanio, 1, bloques_dat);

    free(datos_a_escribir);
}

void leer_desde_archivo_dialfs(char* nombre_archivo, int direccion, int tamanio, int ubicacionPuntero, int pid) {

    int bloque_inicial, tamanio_archivo;

    leer_metadata(nombre_archivo, &bloque_inicial, &tamanio_archivo);
    
    if (ubicacionPuntero + tamanio > tamanio_archivo) {
        // log_error(muy mal)
        exit(-1);
    }

    void* datos_a_leer = malloc(tamanio);

    fseek(bloques_dat, bloque_inicial*block_size + ubicacionPuntero, SEEK_SET);

    fread(datos_a_leer, tamanio, 1, bloques_dat);

    enviar_a_memoria_para_escribir((uint32_t)direccion, datos_a_leer, (uint32_t)tamanio, pid);

    free(datos_a_leer);
}
//ARCHIVOS

void _printearBitarray() {
    for(int i = 0; i < bitarray_get_max_bit(bitmap); i++) {
        printf("%d", bitarray_test_bit(bitmap, i));
    }
    printf("\n");
}

void iniciarInterfazDialFS(t_config* config, char* nombre){
    int tiempo_pausa = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");
    block_size = config_get_int_value(config, "BLOCK_SIZE");
    block_count = config_get_int_value(config, "BLOCK_COUNT");
    retraso_compactacion = config_get_int_value(config, "RETRASO_COMPACTACION");
    path_base_dialfs = config_get_string_value(config, "PATH_BASE_DIALFS");
    int tam_bloq_dat = block_size*block_count;

    cargar_lista_archivos();

    abrir_bloques_dat();

    cargar_bitmap();
    crear_archivo_en_dialfs("goku",4);
    crear_archivo_en_dialfs("vegetta",2);

    _printearBitarray();

    eliminar_archivo_en_dialfs("goku");
    
    _printearBitarray();

    truncar_archivo_en_dialfs("vegetta", 4, 0);

    _printearBitarray();

    truncar_archivo_en_dialfs("vegetta", 6, 0);

    _printearBitarray();
    /*

    crear_archivo_en_dialfs("piccoro",1024);

    truncar_archivo_en_dialfs("goku", 512, 100);
    */

    guardar_bitmap();

    guardar_lista_archivos();



    return;

    t_paquete* paquete = crear_paquete(CONEXION_DIAL_FS);
    enviar_paquete(paquete, conexion_memoria);
    eliminar_paquete(paquete);

    while (1) {
        op_code reciv = recibir_operacion(conexion_kernel);

        if (reciv < 0) {
            exit(-1);
        }
        t_buffer* buffer = recibir_buffer(conexion_kernel);

        int pid = buffer_read_int(buffer);
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
                int direccion = buffer_read_int(buffer);
                int tamanio = buffer_read_int(buffer);
                int ubicacionPuntero = buffer_read_int(buffer);
                escribir_en_archivo_dialfs(nombre_archivo, direccion, tamanio, ubicacionPuntero, pid);
                free(nombre_archivo);
                break;

            case ENVIAR_DIALFS_READ:
                direccion = buffer_read_int(buffer);
                tamanio = buffer_read_int(buffer);
                ubicacionPuntero = buffer_read_int(buffer);
                leer_desde_archivo_dialfs(nombre_archivo, direccion, tamanio, ubicacionPuntero, pid);
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