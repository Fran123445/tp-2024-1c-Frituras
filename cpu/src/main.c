#include <stdlib.h>
#include <stdio.h>
#include <utils/client.h>
#include <utils/server.h>

int main(int argc, char* argv[]) {
    t_config* nuevo_config = config_create("cpu.config");
    if (nuevo_config == NULL) {
    exit(1);
    }; 

    conectarse_a(nuevo_config,"IP_MEMORIA","PUERTO_MEMORIA");
    
    return 0;
}
