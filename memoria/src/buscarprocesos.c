#include "buscarprocesos.h"

bool comparar_pid(int num, t_proceso* proceso){
    return num == proceso->pid;
}