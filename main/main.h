#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <spiffs_partition.h>
#include <freertos/queue.h>
#include "driver/uart.h"
#include <string.h>
#include <dron.h>

#define STACK_SIZE 1024                     /* Memoria para la ejecución de tareas en paralelo. */
                                            /*
                                            * Máximo de muestras por variable ( 4 estados + 4 setpoints = 8 variables ).
                                            * 
                                            * - Tamaño de la partición SPIFFS: 956561 bytes.
                                            * - Tamaño de cada dato: 251 bytes.
                                            * - Cantidad de muestras por variable = [ ( 956561 /  251 )  / 8 ] = 436 datos.
                                            * - Total de bytes usados = 436 [ datos ] * 251 [ bytes / dato ] * 8 variables = 875488 bytes.
                                            * 
                                            * */
#define SAMPLES 10
#define BUFF_SIZE 1024                      /* Tamaño en bytes de los datos transferidos por UART. */
#define SEND_DATA_CMD "send"                /* Comando para transferir datos de la memoria flash al puerto serie. */
#define BAUD_RATE 115200                    /* Tasa de transferencia de bits/segundo. */


#endif
