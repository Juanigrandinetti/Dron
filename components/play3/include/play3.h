#ifndef PLAY3_H
#define PLAY3_H


#include <ps3.h>
#include <change_mac.h>
#include <pwm.h>
#include <controladores.h>


#define PS3_TAG "MANDO"


void controller_event_cb( ps3_t ps3, ps3_event_t event );
init_error_t joystick_init(struct Joystick *self);
void new_joystick(struct Joystick *self, const uint8_t mac_address[MAC_ADDR_SIZE]);


#endif
