#include <stdio.h>
#include <controladores.h>


Pid pid[ 4 ];
Setpoint sp;


float get_pid( struct Pid* self, float error )
{
    self->_p = self->kp * error;                                              /* P = kp * error. */

    self->integral = self->integral + ( error * dt );
    self->_i = self->integral * self->ki;                                     /* I = ki * integral del error. */

    self->_d = error * ( N_z / ( 1 + ( N_z * self->integral ) ) * self->kd ); /* D = kd * derivada del error. */
    
    return ( self->_p + self->_i + self->_d );                                /* Acción de control = P + I + D. */
}


void update_pid( struct Pid ( *pid )[ 4 ], struct Mma* mma, struct Dron* dron )
{
    if ( dron->z != -1 )
    {
        float error_z     = sp.z     - dron->z;
        float error_roll  = sp.roll  - dron->roll;
        float error_pitch = sp.pitch - dron->pitch;
        float error_yaw   = sp.yaw   - dron->yaw;

        mma->pid_z     = pid[ 0 ]->get_pid( &pid[ 0 ], error_z );
        mma->pid_roll  = pid[ 1 ]->get_pid( &pid[ 1 ], error_roll );
        mma->pid_pitch = pid[ 2 ]->get_pid( &pid[ 2 ], error_pitch );
        mma->pid_yaw   = pid[ 3 ]->get_pid( &pid[ 3 ], error_yaw );
    }
    
    else
        ESP_LOGE( TAG, "Error al medir distancia." );
}


void new_pid( struct Pid* self, float kp, float ki, float kd )
{
    /*
    * Atributos
    * ---------
    * 
    * */
    self->kp = kp;
    self->ki = ki;
    self->kd = kd;
    self->_p = 0;
    self->_i = 0;
    self->_d = 0;
    
    /*
    * Métodos
    * -------
    * 
    * */
    self->integral   = 0;
    self->update_pid = update_pid;
    self->get_pid    = get_pid;
}


void new_sp( struct Setpoint* self )
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
}
