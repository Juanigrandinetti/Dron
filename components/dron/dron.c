#include <stdio.h>
#include "dron.h"


const uint8_t mac_joystick[ MAC_ADDR_SIZE ] = { 0xF0, 0xF0, 0x02, 0x43, 0x53, 0x53 };   /* Dirección MAC del Joystick. */
Joystick js;                                                                            /* Variable utilizada para instanciar un objeto de clase Joystick. */
Mma mma;                                                                                /* Variable utilizada para instanciar un objeto de clase Mma. */
Hcsr04 hcsr04;                                                                          /* Variable utilizada para instanciar un objeto de clase Hcsr04. */


init_error_t dron_init( struct Dron* self )
{
    /*
    * Instanciar objetos
    * ------------------
    * 
    * Para instanciar "objetos" es necesario llamar a la función new_...
    * 
    * */

    new_pwm( &motores[ 0 ], GPIO_PWM_00A, MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM0A );
    /* new_pwm( &motores[ 1 ], GPIO_PWM_00B, MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM0B ); */
    /* new_pwm( &motores[ 2 ], GPIO_PWM_01A, MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM0A ); */
    /* new_pwm( &motores[ 3 ], GPIO_PWM_01B, MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM0B ); */
    new_hcsr04( &hcsr04, TRIG, ECHO );
    new_joystick( &js, mac_joystick );
    new_sp( &sp );
    new_mma( &mma );
    new_pid( &pid[ 0 ], Kp_z, Ki_z, Kd_z );
    new_pid( &pid[ 1 ], Kp_roll, Ki_roll, Kd_roll );
    new_pid( &pid[ 2 ], Kp_pitch, Ki_pitch, Kd_pitch );
    new_pid( &pid[ 3 ], Kp_yaw, Ki_yaw, Kd_yaw );

    if (hcsr04.init( &hcsr04 ) != INIT_OK )
    {
        ESP_LOGE( TAG, "Error al inicializar el sensor HC-SR04" );
        return INIT_FAIL;
    }

    if ( js.joystick_init( &js ) != INIT_OK )
    {
        ESP_LOGE( TAG, "Error al modificar la dirección MAC del joystick." );
        return INIT_FAIL;
    }

    for (int i = 0; i < 1; i++)
    {
        init_error_t motor_init = motores[ i ].init( &motores[ i ] );
        if ( motor_init != INIT_OK )
        {
            switch( motor_init )
            {
                case PWM_GPIO_ERROR:
                    ESP_LOGE( TAG, "Error al inicializar los GPIO PWM." );
                    return INIT_FAIL;
                case PWM_CFG_ERROR:
                    ESP_LOGE( TAG, "Error al configurar las unidades PWM." );
                    return INIT_FAIL;
            }
        }    
    }

    return INIT_OK;
}


void update_distance( struct Dron* self )
{
    self->z = hcsr04.get_distance( &hcsr04 );
}


void update_angles( struct Dron* self )
{

}


void updatedc( struct Dron* self )
{
    /*
    * Esta función no sirve hasta no programar la clase Mpu.
    * 
    pid->update_pid( &pid, &mma, hcsr04.distancia, mpu.roll, mpu, pitch, mpu yaw );
    * 
    * */
    motores->update_DC( &mma, &motores );
}


init_error_t state( struct Dron* self )
{
    return INIT_OK;
}


void new_dron( struct Dron* self )
{
    /*
    * Atributos
    * ---------
    * 
    * */
    self->z     = 0;
    self->roll  = 0;
    self->pitch = 0;
    self->yaw   = 0;

    /*
    * Métodos
    * -------
    * 
    * */
    self->init            = dron_init;
    self->update_distance = update_distance;
    self->update_angles   = update_angles;
    self->updatedc        = updatedc;
    self->state           = state;
}
