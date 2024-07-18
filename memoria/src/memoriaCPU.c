#include <commons/collections/list.h>
#include <utils/serializacion.h>
#include <utils/pcb.h>
#include <unistd.h>
#include <commons/config.h>
#include <math.h>
#include "memoriaKernel.h"
#include "main.h"
#include "memoriaDeInstrucciones.h"
#include "memoriaContigua.h"

pthread_mutex_t mutex_bitarray_marcos_libres = PTHREAD_MUTEX_INITIALIZER;

t_list* sacar_n_entradas_desde_final(int cant_pags_a_sacar, t_list* tabla_del_proceso){
    int tamanio_tabla = list_size(tabla_del_proceso);
    
    if(cant_pags_a_sacar > tamanio_tabla){
        fprintf(stderr, "Se quiere sacar mas paginas que las que hay actualmente");
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < cant_pags_a_sacar; i++){
        tamanio_tabla = list_size(tabla_del_proceso);
        informacion_de_tabla* entrada = list_remove(tabla_del_proceso,tamanio_tabla - 1); 
        free(entrada);
    }

    return tabla_del_proceso;
}

void marcar_frames_como_libres(int cant_pags_a_sacar, t_list* tabla_del_proceso){
    int tamanio_tabla = list_size(tabla_del_proceso);

    for(int i = tamanio_tabla - 1; i>=tamanio_tabla - cant_pags_a_sacar; i--){
        informacion_de_tabla* entrada = list_get(tabla_del_proceso, i);
        pthread_mutex_lock(&mutex_bitarray_marcos_libres);
        if(entrada->validez){
            bitarray_clean_bit(mapa_de_marcos,entrada->marco);
        }
        pthread_mutex_unlock(&mutex_bitarray_marcos_libres);
    }
}

void chequear_espacio_memoria (int cant_pags, int socket_cpu){

    if(!memoria_contigua){
        fprintf(stderr, "No se creó memoria contigua aún");
        exit(EXIT_FAILURE);
    }

    int contador_marcos_libres = 0;
    pthread_mutex_lock(&mutex_bitarray_marcos_libres);
    for (size_t i = 0; i < (mapa_de_marcos->size); i++) {
        if (!bitarray_test_bit(mapa_de_marcos, i)) {
            contador_marcos_libres++;
        }
    }
    if (contador_marcos_libres < cant_pags){
        fprintf(stderr, "No hay suficientes marcos disponibles");
        //log_info(log_memoria, "OUT OF MEMORY, SE FINALIZARÁ LA EJECUCIÓN DEL PROCESO.");
        t_paquete* paquete = crear_paquete(OUT_OF_MEMORY);
        enviar_paquete(paquete, socket_cpu);
        eliminar_paquete(paquete);
        exit(EXIT_FAILURE);
    }
    pthread_mutex_unlock(&mutex_bitarray_marcos_libres);
}

void asignar_frames_a_paginas (int cant_pags_total, t_proceso_memoria* proceso){
    int cont_marcos_asignados = 0;
    int pag_actual = 0;

    pthread_mutex_lock (&mutex_bitarray_marcos_libres);

    for(int i = 0; i<mapa_de_marcos->size; i++){
        if(!bitarray_test_bit(mapa_de_marcos, i)){
            informacion_de_tabla* tabla = malloc(sizeof(informacion_de_tabla));

            if (tabla == NULL){
                fprintf(stderr, "No hay memoria dispo para la tabla"); 
                pthread_mutex_unlock(&mutex_bitarray_marcos_libres);
                exit(EXIT_FAILURE);
            }

            bitarray_set_bit(mapa_de_marcos, i);
            tabla->marco = i;
            tabla-> validez = true;
            list_add(proceso->tabla_del_proceso, tabla);
            cont_marcos_asignados++;
            pag_actual++;

            if (cont_marcos_asignados == cant_pags_total){
                break;
            }
        }
   }
   pthread_mutex_unlock (&mutex_bitarray_marcos_libres);
}

t_proceso_memoria* recibir_proceso_cpu(int socket_cpu){
    t_proceso_memoria* proceso_cpu = malloc(sizeof(t_proceso_memoria));
    t_buffer* buffer = recibir_buffer(socket_cpu);
    int pid = buffer_read_int(buffer);
    uint32_t pc = buffer_read_uint32(buffer);
    liberar_buffer(buffer);
    proceso_cpu->pid = pid;
    proceso_cpu->pc= pc;
    return proceso_cpu;
}

void mandar_instruccion_cpu(int socket_kernel, int socket_cpu){
    t_proceso_memoria* proceso = recibir_proceso_cpu(socket_cpu);
    if (proceso == NULL){
        fprintf(stderr, "Error al recibir proceso desde CPU");
    }

    char* instruccion = obtener_instruccion(socket_kernel,(proceso->pc), (proceso->pid));
    if(instruccion == NULL){
        fprintf(stderr, "Error de la instruccion obtenida");
        free(proceso);
        return;
    }

    usleep(tiempo_retardo * 1000);

    t_paquete* paquete = crear_paquete(ENVIO_DE_INSTRUCCIONES);
    agregar_string_a_paquete(paquete,instruccion);
    enviar_paquete(paquete, socket_cpu);

    eliminar_paquete(paquete);
    free(proceso);
}

void* resize_proceso(int socket_cpu){
    t_buffer* buffer = recibir_buffer(socket_cpu);
    int pid = buffer_read_int(buffer);
    int tamanio_nuevo = buffer_read_int(buffer);

    t_proceso_memoria* proceso = hallar_proceso(pid);
    t_paquete* paquete;

    if(proceso->tamanio_proceso < tamanio_nuevo){
        if(tamanio_nuevo > tam_memoria){
            fprintf(stderr, "Error: se pide mas memoria que la que hay");
            log_info(log_memoria, "OUT OF MEMORY, SE FINALIZARÁ EL PROCESO.");
            paquete = crear_paquete(OUT_OF_MEMORY);
            enviar_paquete(paquete, socket_cpu);

            eliminar_paquete(paquete);
            liberar_buffer(buffer);
            pthread_mutex_destroy(&mutex_bitarray_marcos_libres);
            free(escucha_cpu);
            free(escucha_kernel);
            free(escucha_io);
            free(memoria_contigua);
            free(bitarray_memoria_usuario);
            bitarray_destroy(mapa_de_marcos);
            list_destroy(lista_de_procesos);
            log_destroy(log_memoria);
            close(socket_servidor_memoria);
            log_destroy(log_servidor);

            config_destroy(config);

            exit(EXIT_FAILURE);
        }
        int cant_paginas_viejas = ceil((float)proceso->tamanio_proceso/tam_pagina);
        int cant_paginas_nuevas = ceil(((float)tamanio_nuevo/tam_pagina));
        int total_paginas_a_agregar = cant_paginas_nuevas - cant_paginas_viejas;
        chequear_espacio_memoria(total_paginas_a_agregar, socket_cpu);

        usleep(tiempo_retardo * 1000);

        log_info(log_memoria, "Ampliacion Proceso - PID: %d - Tamanio Actual: %d - Tamaño a Ampliar: %d", pid, proceso->tamanio_proceso , tamanio_nuevo);

        proceso->tamanio_proceso = tamanio_nuevo;

        asignar_frames_a_paginas(total_paginas_a_agregar,proceso);

    }else if (proceso->tamanio_proceso > tamanio_nuevo){
        log_info(log_memoria, "Reduccion Proceso - PID: %d - Tamanio Actual: %d - Tamaño a Reducir: %d", pid, proceso->tamanio_proceso , tamanio_nuevo);

        usleep(tiempo_retardo * 1000);

        int paginas_viejas = ceil((float)proceso->tamanio_proceso/tam_pagina);
        int paginas_nuevas = ceil((float)tamanio_nuevo/tam_pagina);
        int total_paginas_a_sacar = paginas_viejas - paginas_nuevas;
        marcar_frames_como_libres(total_paginas_a_sacar, proceso->tabla_del_proceso);
        proceso->tabla_del_proceso = sacar_n_entradas_desde_final(total_paginas_a_sacar, proceso->tabla_del_proceso);          
        proceso->tamanio_proceso = tamanio_nuevo;

    }else{
        log_info(log_memoria, "Resize pedido es el tamanio que ya tiene el proceso");
        liberar_buffer(buffer);
        return NULL;
    }

    paquete = crear_paquete(RESIZE_ACEPTADO);
    enviar_paquete(paquete,socket_cpu);

    eliminar_paquete(paquete);
    liberar_buffer(buffer); 
}

void* acceso_tabla_paginas(int socket_cpu){
    t_buffer* buffer = recibir_buffer(socket_cpu);
    int pid = buffer_read_int(buffer);
    int pagina_a_buscar = buffer_read_int(buffer);
    t_proceso_memoria* proceso = hallar_proceso(pid);

    if(proceso == NULL){
        fprintf(stderr, "No se encuentra el PID en la lista de procesos");
        free(proceso);
        liberar_buffer(buffer);
        return NULL;
    }
    if(pagina_a_buscar < 0 || pagina_a_buscar >= list_size(proceso->tabla_del_proceso)){
        fprintf(stderr, "Error: Nro de página no válido");
        free(proceso);
        liberar_buffer(buffer);
        return NULL;
    }

    usleep(tiempo_retardo * 1000);

    informacion_de_tabla* entrada = list_get(proceso->tabla_del_proceso, pagina_a_buscar);
    int marco = entrada->marco;

    t_paquete* paquete = crear_paquete(ACCESO_TABLAS_PAGINAS);
    agregar_int_a_paquete(paquete, marco);
    enviar_paquete(paquete, socket_cpu);
    
    liberar_buffer(buffer);
    eliminar_paquete(paquete);
}