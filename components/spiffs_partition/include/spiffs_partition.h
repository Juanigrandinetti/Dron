#ifndef SPIFFS_PARTITION_H
#define SPIFFS_PARTITION_H
#include <estructuras.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>


void new_spiffs( struct Flash* self );
void restart( void );
void spiffs_cfg( struct Flash* self );
void spiffs_info( struct Flash* );
const esp_partition_t* get_partition( struct Flash* self );
esp_err_t erase_partition( struct Flash* self, const esp_partition_t* partition, size_t starting_position, size_t size_to_erase );


#endif
