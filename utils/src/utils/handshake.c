#include "handshake.h"

int compararHandshake(int handshake_escucha, int handshake_recibido) {
    if (handshake_escucha == MEMORIA) {
        return handshake_recibido != MEMORIA;
    } else {
        switch(handshake_escucha) {
            case CPU:
                return handshake_recibido == KERNEL;
            case KERNEL:
                return handshake_recibido == IO;
            default:
                return false;
        }
    }
}