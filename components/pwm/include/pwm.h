#ifndef PWM_H
#define PWM_H


#include <driver/mcpwm.h>
#include <esp_log.h>
#include <estructuras.h>


#define MAX_DC 10    // 10% ( máxima velocidad de giro ).
#define MIN_DC 5.8   // 5.8% ( motor apagado ).
#define FREQUENCY 50 // Frecuencia de trabajo de los motores.


extern Pwm motores[1];


init_error_t pwm_init(struct Pwm* self);
void increase_dc(struct Pwm* self);
void decrease_dc(struct Pwm* self);
void new_pwm(struct Pwm* self, int gpio, mcpwm_unit_t unit, mcpwm_timer_t timer, mcpwm_generator_t generator);
void set_dc( struct Pwm* self, float dc );
void update_DC( struct Mma* mma, struct Pwm ( *motores )[ 4 ] );


#endif
