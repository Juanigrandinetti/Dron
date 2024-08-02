#ifndef MAC_H
#define MAC_H


#include <esp_system.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <errores.h>
#include <estructuras.h>


void get_mac_address( struct Mac* self );
init_error_t set_mac_address( struct Mac* self, uint8_t *mac );
void nvs_ini( struct Mac* self );
void new_mac( struct Mac* self );


#endif
