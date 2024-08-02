#include <stdio.h>
#include <pwm.h>


Pwm motores[ 1 ];


init_error_t pwm_init( struct Pwm* self )
{
    ESP_LOGI( TAG, "Inicializando GPIO PWM..." );
    
    /*
    Nombre: "mcpwm_gpio_init".
    Inicializar la señal PWM "0" de la unidad "A".
    MCPWM_UNIT_0: elección del canal (opciones: 0/1).
    MCPWM0A: setear la señal PWM (opciones: 6 salidas PWM por cada unidad (A/B).
    GPIO_PWM: pin PWM a utilizar ().
    */

    if ( mcpwm_gpio_init( self->unit, self->generator, self->gpio ) != ESP_OK )
        return PWM_GPIO_ERROR;

    ESP_LOGI( TAG, "GPIO PWM inicializados." );

    self->cfg.frequency = FREQUENCY;
    self->cfg.cmpr_a = MIN_DC;
    self->cfg.duty_mode = MCPWM_DUTY_MODE_0;
    self->cfg.counter_mode = MCPWM_UP_COUNTER;
    
    /*
    Nombre: "mcpwm_init".
    Inicializar parámetros MCPWM.
    MCPWM_UNIT_0: elección del canal (opciones: 0/1).
    MCPWM_TIMER_0: elección del timer  (opciones: 0/1/2), cada unidad (A/B) posee 3 timers asociados.
    pwm_config: estructura con configuraciones PWM.
    */

    ESP_LOGI( TAG, "Inicializando unidades PWM..." );

    if ( mcpwm_init( self->unit, self->timer, &( self->cfg ) ) != ESP_OK )
        return PWM_CFG_ERROR;

    ESP_LOGI( TAG, "Unidades PWM inicializadas." );

    return INIT_OK;
}


void increase_dc( struct Pwm* self )
{
    if ( mcpwm_get_duty( self->unit, self->timer, self->generator ) < MAX_DC )
    { 
        mcpwm_set_duty( self->unit, self->timer, self->generator, mcpwm_get_duty( self->unit, self->timer, self->generator ) + 0.01 );
        ESP_LOGI( TAG, "DC: %.2f\n", mcpwm_get_duty( self->unit, self->timer, self->generator ) );
    }
}


void decrease_dc( struct Pwm* self )
{
    if ( mcpwm_get_duty( self->unit, self->timer, self->generator ) > MIN_DC )
    {
        mcpwm_set_duty( self->unit, self->timer, self->generator, mcpwm_get_duty( self->unit, self->timer, self->generator ) - 0.01 );
        ESP_LOGI( TAG, "DC: %.2f\n", mcpwm_get_duty( self->unit, self->timer, self->generator ) );
    }
}


void set_dc( struct Pwm* self, float dc )
{
    mcpwm_set_duty( self->unit, self->timer, self->generator, dc );
    ESP_LOGI( TAG, "DC: %.2f\n", mcpwm_get_duty( self->unit, self->timer, self->generator ) );
}


void update_DC( struct Mma* mma, struct Pwm ( *motores )[ 4 ] )
{
    mma->update_dc( &mma );
    motores[ 0 ]->set_dc( &motores[ 0 ], mma->dc1 );
    motores[ 1 ]->set_dc( &motores[ 1 ], mma->dc2 );
    motores[ 2 ]->set_dc( &motores[ 2 ], mma->dc3 );
    motores[ 3 ]->set_dc( &motores[ 3 ], mma->dc4 );
}


void new_pwm( Pwm* self, int gpio, mcpwm_unit_t unit, mcpwm_timer_t timer, mcpwm_generator_t generator )
{
    self->gpio = gpio;
    self->unit = unit;
    self->timer = timer;
    self->generator = generator;
    self->init = pwm_init;
    self->increase_dc = increase_dc;
    self->decrease_dc = decrease_dc;
}