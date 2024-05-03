#include "registroCpu.h"

registros_cpu miCPU; 

void inicializar_registros_cpu() {
    miCPU.AX = 0;
    miCPU.BX = 0;
    miCPU.CX = 0;
    miCPU.DX = 0;
    miCPU.EAX = 0;
    miCPU.EBX = 0;
    miCPU.ECX = 0;
    miCPU.EDX = 0;
    miCPU.SI = 0;
    miCPU.DI = 0;
    miCPU.PC = 0;
    printf("Registros de CPU inicializados.\n");
  //miCPU.PC = 0; // Inicializar registros con valores iniciales (opcional)
  // ... inicializar otros registros
}


void* obtenerRegistro(registrosCPU registro) {
    void* lista_de_registros[11] = {
        &miCPU.AX, &miCPU.BX, &miCPU.CX, &miCPU.DX,
        &miCPU.EAX, &miCPU.EBX, &miCPU.ECX, &miCPU.EDX,
        &miCPU.SI, &miCPU.DI, &miCPU.PC
    };

    if(registro >= 0 && registro < 11){
        return(lista_de_registros[registro]);
    }
    else{
        return NULL;
    }
    }

    //listaDirDeRegistros[11] = [&miCPU.AX, &miCPU.BX, &miCPU.CX, &miCPU.DX, &miCPU.EAX, &miCPU.EBX,
    //                           &miCPU.ECX, &miCPU.EDX, &miCPU.SI, &miCPU.DI, &miCPU.PC];
    //return listaDirDeRegistros[registro];
    
    //No estoy seguro de cómo funcionan los Enum así que no se si funciona así, además de que este caso no contempla el "NULL".
    //Pero como decía más arriba, si no van a mandarnos a probar cosas con errores sintácticos, entonces esto es más eficiente.


size_t tamanioRegistro(registrosCPU registro) {
    if (registro >= AX && registro <= DX) {
        return sizeof(uint8_t);
    } else {
        return sizeof(uint32_t);
    }
    // En las funciones que invocan a esto, siempre antes se verifica que el registro exista, así que no hace falta que lo verifique acá también.
}



