#ifndef HCSR04
#define HCSR04


#include <driver/gpio.h>
#include <esp_log.h>
#include <rom/ets_sys.h>
#include <esp_timer.h>
#include <estructuras.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>


#define V_SONIDO        0.0343    // [cm/us] -------- (343 m/s) * (100 cm/m) / (1 s/1000000 us) = 0.03443 cm/us.
#define ECHO_TIMEOUT    6000      // us.
#define LOW_TRIG_DELAY  4         // us.
#define HIGH_TRIG_DELAY 10        // us.


void gpio_init( int pin, gpio_mode_t mode );
init_error_t hcsr04_init( struct Hcsr04* self );
esp_err_t measure_Distance( int64_t *time, int trig_pin, int echo_pin );
float get_distance( struct Hcsr04* self  );
void new_hcsr04( Hcsr04* self, int trig, int echo );


#endif
