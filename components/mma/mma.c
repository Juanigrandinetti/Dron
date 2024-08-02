#include <stdio.h>
#include "mma.h"


static float get_dc( float pid_e )
{
    if ( pid_e > MAX_DC )
        return MAX_DC;

    else
    {
        if ( pid_e < MIN_DC )
            return MIN_DC;

        else
            return pid_e;
    }
}


void update_dc( struct Mma* self )
{
    
    float roll_pos = self->pid_roll * 0.5;
    float roll_neg = self->pid_roll * ( -0.5 );

    float pitch_pos = self->pid_pitch * 0.5;
    float pitch_neg = self->pid_pitch * ( -0.5 );

    float yaw_pos = self->pid_yaw * 0.5;
    float yaw_neg = self->pid_yaw * ( -0.5 );

   self->dc1 = get_dc( ( self->pid_z + pitch_neg ) + ( roll_pos + yaw_pos ) );
   self->dc2 = get_dc( ( self->pid_z + pitch_neg ) + ( roll_neg + yaw_neg ) );
   self->dc3 = get_dc( ( self->pid_z + pitch_pos ) + ( roll_neg + yaw_pos ) );
   self->dc4 = get_dc( ( self->pid_z + pitch_pos ) + ( roll_pos + yaw_neg ) );
}


void new_mma( struct Mma* self )
{
    self->pid_z = 0;
    self->pid_roll = 0;
    self->pid_pitch = 0;
    self->pid_yaw = 0;
    self->dc1 = 0;
    self->dc2 = 0;
    self->dc3 = 0;
    self->dc4 = 0;
    self->update_dc = update_dc;
}
