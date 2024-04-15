#include <stdlib.h>
#include <stdio.h>
#include <utils/client.h>
#include <utils/server.h>

int main(int argc, char* argv[]) {
    t_config* nuevo_config = config_create("entradasalida.config");
    if (nuevo_config == NULL) {
    exit(1);
    }; 

    conectarse_a(nuevo_config,"IP_KERNEL","IP_KERNEL");
    
    return 0;
}
