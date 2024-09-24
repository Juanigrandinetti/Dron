#ifndef PLAY3_H
#define PLAY3_H


#include <ps3.h>
#include <change_mac.h>
#include <pwm.h>
#include <controladores.h>
#include <driver/gpio.h>


#define PS3_TAG "MANDO"

extern Dron dron;

/*
Nombre: controller_event_cb

Descripción: manejar las interrupciones asociadas
             a eventos del joystick.

Devuelve: N/A.
*/
void controller_event_cb( ps3_t ps3, ps3_event_t event );

/*
Nombre: joystick_init

Descripción: inicializa la dirección MAC del ESP32 con la dirección MAC precargada
             del Joystick. A su vez, establece la conexión entre el Joystick y la
             ESP32.

Devuelve: init_error_t.
*/
init_error_t joystick_init(struct Joystick *self);

void new_joystick(struct Joystick *self, const uint8_t mac_address[MAC_ADDR_SIZE]);


#endif
