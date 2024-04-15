#include <stdlib.h>
#include <stdio.h>
#include <conexiones.h>
#include <utils/client.h>
#include <utils/server.h>




int main(int argc, char* argv[]) {
    t_config* nuevo_config = config_create("kernel.config");
    if (nuevo_config == NULL) {
    exit(1);
    }; 

    t_conexion* cpu = {
        nuevo_config,
        "IP_CPU",
        "PUERTO_ESCUCHA_DISPATCH"
    };

    conectarse_a(cpu);
    return 0;
}
