#include "registroCpu.h"

registros_cpu miCPU; //Reserva memoria para la variable global miCPU. 
                     //Este espacio de memoria es esencial para almacenar 
                     //los valores reales de los registros de CPU que representa la variable.
                     //(esto me lo propuso chatGPT, yo no lo termino de entender)
                     //¿por qué declarla otra vez si en el header ya existe la variable global?

void init_cpu_registers() {
  //myCPU.PC = 0; // Inicializar registros con valores iniciales (opcional)
  // ... inicializar otros registros
}