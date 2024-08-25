#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H


#include <errores.h>
#include <driver/mcpwm.h>
#include <esp_spiffs.h>
#include <esp_partition.h>


#define MAC_ADDR_SIZE 6 // Cantidad de bytes de la dirección MAC.
#define TAG "DRON"


typedef struct Dron
{
    /*
    * Atributos
    * ---------
    * 
    * */
    float z;
    float roll;
    float pitch;
    float yaw;

    /*
    * Métodos
    * -------
    * 
    * */
    init_error_t ( *init )( struct Dron* );
    void ( *update_distance )( struct Dron* );
    void ( *update_angles )( struct Dron* );
    void ( *updatedc )( struct Dron* );
    init_error_t ( *state )( struct Dron* );
}Dron;


typedef struct Flash
{
    /*
    * Atributos
    * ---------
    * 
    * */
    esp_vfs_spiffs_conf_t cfg;
    size_t total;
    size_t used;

    /*
    * Métodos
    * -------
    * 
    * */
    void ( *spiffs_cfg )( struct Flash* );
    void ( *spiffs_info )( struct Flash* );
    const esp_partition_t* ( *get_partition )( struct Flash* );
    esp_err_t ( *erase_partition )( struct Flash*, const esp_partition_t*, size_t, size_t );
    esp_err_t ( *read )( struct Flash*, const char* );
    int ( *write )( struct Flasgh*, const char*, int, int* );
}Flash;


typedef struct Joystick
{
    /*
    * Atributos
    * ---------
    * 
    * */
    uint8_t mac[ MAC_ADDR_SIZE ];

    /*
    * Métodos
    * -------
    * 
    * */
    init_error_t ( *joystick_init )( struct Joystick* );
}Joystick;


typedef struct Hcsr04
{
    /*
    * Atributos
    * ---------
    * 
    * */
    int trig;
    int echo;
    float distancia;

    /*
    * Métodos
    * -------
    * 
    * */
    init_error_t ( *init )( struct Hcsr04* );
    float ( *get_distance )( struct Hcsr04* );
}Hcsr04;


typedef struct Pwm
{
    /*
    * Atributos
    * ---------
    * 
    * */
    int gpio;
    mcpwm_unit_t unit;
    mcpwm_timer_t timer;
    mcpwm_generator_t generator;
    mcpwm_config_t cfg;

    /*
    * Métodos
    * -------
    * 
    * */
    init_error_t ( *init )( struct Pwm* );
    void ( *increase_dc )( struct Pwm* );
    void ( *decrease_dc )( struct Pwm* );
    void ( *set_dc )( struct Pwm*, float );
    float ( *update_DC )( struct Mma*, struct Pwm* );
}Pwm;


typedef struct Mac
{
    /*
    * Métodos
    * -------
    * 
    * */
    void ( *get_mac_address )( struct Mac* );
    init_error_t ( *set_mac_address )( struct Mac*, uint8_t* );
    void ( *nvs_init )( struct Mac* );
}Mac;


typedef struct Pid
{
    /*
    * Atributos
    * ---------
    * 
    * */
    float kp;
    float ki;
    float kd;
    float _p;
    float _i;
    float _d;
    float integral;

    /*
    * Métodos
    * -------
    * 
    * */
    float ( *update_pid )( struct Pid*, struct Mma*, struct Dron* );
    float ( *get_pid )( struct Pid*, float );
}Pid;


typedef struct Mma
{
    /*
    * Atributos
    * ---------
    * 
    * */
    float pid_z;
    float pid_roll;
    float pid_pitch;
    float pid_yaw;
    float dc1;
    float dc2;
    float dc3;
    float dc4;

    /*
    * Métodos
    * -------
    * 
    * */
    void ( *update_dc )( struct Mma* );
}Mma;



typedef struct Setpoint
{
    /*
    * Atributos
    * ---------
    * 
    * */
    float z;
    float roll;
    float pitch;
    float yaw;
}Setpoint;


#endif
