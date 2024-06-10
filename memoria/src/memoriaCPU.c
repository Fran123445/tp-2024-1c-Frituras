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

void agregar_n_entradas_vacias(t_list *list, int n) {
    for (int i = 0; i < n; i++) {
        list_add(list, NULL);
    }
}

void chequear_espacio_memoria (int cant_pags, int socket_cpu){

    if(!memoria_contigua){
        fprintf(stderr, "No se creó memoria contigua aún")
        exit(EXIT_FAILURE);
    }

    int contador_marcos_libres = 0;

    for (size_t i = 0; i < mapa_de_marcos->size; i++) {
    if (!bitarray_test_bit(mapa_de_marcos, i)) {
        contador_marcos_libres++;
    }
        }

    if (contador_marcos_libres < cant_pags){
        fprintf(stderr, "No hay suficientes marcos disponibles");
        t_paquete* paquete = crear_paquete(OUT_OF_MEMORY);
        enviar_paquete(paquete, socket_cpu);
        exit(EXIT_FAILURE);
    }
}

void* asignas_frames_a_paginas (int cant_pags, int pid, int socket_cpu){
   
    memset(memoria_contigua, 0, tamanio);
    
    size_t marcos_asignados = 0;
    for(size_t i = 0; i < mapa_de_marcos->size; i++){
        if(!bitarray_test_bit(mapa_de_marcos, i)){
            bitarray_set_bit(mapa_de_marcos,i);
            marcos_asignados++;
            if(marcos_asignados == cant_pags){
                break;
            }
        }
    }
    t_proceso_memoria* proceso = hallar_proceso(pid);


    return memoria_contigua;
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
            chequear_espacio_memoria(paginas_nuevas);
            proceso->tamanio_proceso = tamanio_nuevo;
            proceso->tabla_del_proceso = agregar_n_entradas_vacias(proceso->tabla_del_proceso, paginas_nuevas);
            memoria_contigua = copiar_pags_vacias_memoria_contigua(paginas_nuevas);
            t_paquete* paquete = crear_paquete(RESIZE_ACEPTADO);
            enviar_paquete(paquete, socket_cpu);
            
            }else if (proceso->tamanio_proceso >tamanio_nuevo){
            proceso->tamanio_proceso = tamanio_nuevo;

        }else{
            return;
        }
        
        liberar_buffer(buffer);


}