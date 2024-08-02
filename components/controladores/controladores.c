#include <stdio.h>
#include <controladores.h>


Setpoint sp;


float get_pid( struct Pid* self, float error )
{
    self->_p = self->kp * error;                                      // P = kp * error.

    self->integral = self->integral + ( error * dt );
    self->_i = self->integral * self->ki;                             // I = ki * integral del error.

    self->_d = ( N_z / ( 1 + ( N_z * self->integral ) ) * self->kd ); // D = kd * derivada del error.
    
    return error * ( self->_p + self->_i + self->_d );                // error = error * ( P + I + D ).
}


void update_pid( struct Pid ( *pid )[ 4 ], struct Mma* mma, float z, float roll, float pitch, float yaw )
{
    if ( z != -1 )
    {
        float error_z = sp.z - z;
        float error_roll = sp.roll - roll;
        float error_pitch = sp.pitch - pitch;
        float error_yaw = sp.yaw - yaw;

        mma->pid_z = pid[ 0 ]->get_pid( &pid[ 0 ], error_z );
        mma->pid_roll = pid[ 1 ]->get_pid( &pid[ 1 ], error_roll );
        mma->pid_pitch = pid[ 2 ]->get_pid( &pid[ 2 ], error_pitch );
        mma->pid_yaw = pid[ 3 ]->get_pid( &pid[ 3 ], error_yaw );
    }
    
    else
        ESP_LOGE( TAG, "Error al medir distancia." );
}


void new_pid( struct Pid* self, float kp, float ki, float kd )
{
    self->kp = kp;
    self->ki = ki;
    self->kd = kd;
    self->_p = 0;
    self->_i = 0;
    self->_d = 0;
    self->integral = 0;
    self->update_pid = update_pid;
    self->get_pid = get_pid;
}


void new_sp( struct Setpoint* self )
{
    self->z = 0;
    self->roll = 0;
    self->pitch = 0;
    self->yaw = 0;
}
