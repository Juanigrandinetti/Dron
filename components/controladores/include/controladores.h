#ifndef CONTROLADORES_H
#define CONTROLADORES_H


#include <estructuras.h>
#include <esp_log.h>
#include <mma.h>
#include <math.h>


#define dt        0.01
#define Kp_z      9.6
#define Ki_z      1.83
#define Kd_z      2.52
#define N_z       2 * M_PI * 1000
#define Kp_roll   39
#define Ki_roll   0.0295
#define Kd_roll   18.65
#define N_roll    2 * M_PI * 1000
#define Kp_pitch  22.62
#define Ki_pitch  0.0295
#define Kd_pitch  11.19
#define N_pitch   2 * M_PI * 1000
#define Kp_yaw    15.6
#define Ki_yaw    0.0295
#define Kd_yaw    7.46
#define N_yaw     2 * M_PI * 1000


                        /*
                        * Inicializar un array con 4 objetos de clase Pid.
                        * 
                        * pid[0]: pid_z
                        * pid[1]: pid_roll
                        * pid[2]: pid_pitch
                        * pid[3]: pid_yaw
                        *
                        * */
extern Pid pid[ 4 ];
extern Setpoint sp;


void new_pid( struct Pid* self, float kp, float ki, float kd );
void new_sp( struct Setpoint* self );
float get_pid( struct Pid* self, float error );
void update_pid( struct Pid ( *pid )[ 4 ], struct Mma* mma, struct Dron* dron );


#endif
