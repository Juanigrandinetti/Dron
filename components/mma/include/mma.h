#ifndef MMMA_H
#define MMA_H


#include <estructuras.h>


void new_mma( struct Mma* self );
void update_dc( struct Mma* self );
static float get_dc( float pid_e );


#endif
