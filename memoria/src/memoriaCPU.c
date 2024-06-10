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
t_list* agregar_n_entradas_vacias(int cant_pags_a_agregar, t_list* tabla_del_proceso) {
    for (int i = 0; i < cant_pags_a_agregar; i++) {
        list_add(tabla_del_proceso, NULL);
    }
    return tabla_del_proceso;
}
t_list* sacar_n_entradas_desde_final(int cant_pags_a_sacar, t_list* tabla_del_proceso){
    int tamanio_tabla = list_size(tabla_del_proceso);
    for(int i = 0; i < cant_pags_a_sacar; i++){
        informacion_de_tabla* entrada = list_remove(tabla_del_proceso,tamanio_tabla - 1 -i); 
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
        fprintf(stderr, "No se creó memoria contigua aún")
        exit(EXIT_FAILURE);
    }

    int contador_marcos_libres = 0;
    pthread_mutex_lock(&mutex_bitarray_marcos_libres);
    for (size_t i = 0; i < mapa_de_marcos->size; i++) {
    if (!bitarray_test_bit(mapa_de_marcos, i)) {
        contador_marcos_libres++;
    }
        }
    if (contador_marcos_libres < cant_pags){
        fprintf(stderr, "No hay suficientes marcos disponibles");
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
    op_code cod_op = recibir_operacion(socket_cpu);
    if (cod_op == ENVIO_PC){
        t_proceso_memoria* proceso_cpu = malloc(sizeof(t_proceso_memoria));
        t_buffer* buffer = recibir_buffer(socket_cpu);
        int pid = buffer_read_int(buffer);
        uint32_t pc = buffer_read_uint32(buffer);
        liberar_buffer(buffer);
        proceso_cpu->pid = pid;
        proceso_cpu->pc= pc;
        return proceso_cpu;
    } return NULL;
}

void mandar_instruccion_cpu(int socket_kernel, int socket_cpu, int tiempo_retardo){
    t_proceso_memoria* proceso = recibir_proceso_cpu(socket_cpu);
    if (proceso == NULL){
        fprintf(stderr, "Error al recibir proceso desde CPU");
    }
    char* instruccion = obtener_instruccion(socket_kernel,(proceso->pc), (proceso->pid));
    if(instruccion == NULL){
        fprintf(stderr, "Error de la instruccion obtenida");
        free (proceso);
        return;
    }
    t_paquete* paquete = crear_paquete(ENVIO_DE_INSTRUCCIONES);
    agregar_string_a_paquete(paquete,instruccion);
    enviar_paquete(paquete, socket_cpu);
    eliminar_paquete(paquete);
    free(instruccion);
    free (proceso);
}

void resize_proceso(int socket_cpu,t_config* config){
    op_code cod_op = recibir_operacion(socket_cpu);
    if (cod_op == ENVIO_RESIZE){
        t_buffer* buffer = recibir_buffer(socket_cpu);
        int pid = buffer_read_int(buffer);
        int tamanio_nuevo = buffer_read_int(buffer);
        t_proceso_memoria* proceso = hallar_proceso(pid);
        if(proceso->tamanio_proceso < tamanio_nuevo){
            int paginas_nuevas = ceil(tamanio_nuevo/config_get_int_value(config, "TAM_PAGINA"));
            chequear_espacio_memoria(paginas_nuevas, socket_cpu);
            proceso->tamanio_proceso = tamanio_nuevo;
            proceso->tabla_del_proceso = agregar_n_entradas_vacias(paginas_nuevas, proceso->tabla_del_proceso);
            asignar_frames_a_paginas(paginas_nuevas,proceso);
            t_paquete* paquete = crear_paquete(RESIZE_ACEPTADO);
            enviar_paquete(paquete, socket_cpu);
            eliminar_paquete(paquete);
            }else if (proceso->tamanio_proceso >tamanio_nuevo){
            proceso->tamanio_proceso = tamanio_nuevo;
            int paginas_a_sacar = ceil (tamanio_nuevo/config_get_int_value(config,"TAM_PAGINA"));
            marcar_frames_como_libres(paginas_a_sacar,proceso->tabla_del_proceso);
            proceso->tabla_del_proceso = sacar_n_entradas_desde_final(paginas_a_sacar, proceso->tabla_del_proceso);
            t_paquete* paquete = crear_paquete(RESIZE_ACEPTADO);
            enviar_paquete(paquete,socket_cpu);
            eliminar_paquete(paquete);
        }else{
            return;
        }
        liberar_buffer(buffer);
}