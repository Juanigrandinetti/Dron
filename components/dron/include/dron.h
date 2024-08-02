#ifndef DRON_H
#define DRON_H


#include <play3.h>
#include <hcsr04.h>
#include <controladores.h>


#define TRIG GPIO_NUM_26         // pin 10
#define ECHO GPIO_NUM_25         // pin 9
#define GPIO_PWM_00A GPIO_NUM_15 // pin 35


init_error_t dron_init( struct Dron* self );
void update_distance( struct Dron* self );
void update_angles( struct Dron* self );
void updatedc( struct Dron* self );
init_error_t state( struct Dron* self );
void new_dron( struct Dron* self );


#endif
