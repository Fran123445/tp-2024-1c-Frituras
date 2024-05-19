#include "handshake.h"

int compararHandshake(int handshake_escucha, int handshake_recibido) {
    switch(handshake_escucha) {
        case MEMORIA:
            return handshake_recibido != MEMORIA;
        case CPU:
            return handshake_recibido == KERNEL;
        case KERNEL:
            return handshake_recibido == IO;
        default:
            return false;
    }
}
