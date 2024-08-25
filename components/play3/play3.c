#include <stdio.h>
#include <play3.h>


Mac mac;


/*
Nombre: controller_event_cb

Descripción: manejar las interrupciones asociadas
             a eventos del joystick.

Devuelve: N/A.
*/
void controller_event_cb( ps3_t ps3, ps3_event_t event )
{    
    if ( ps3.analog.button.up )
    {
        sp.z = sp.z + 0.1;
        motores[ 0 ].increase_dc( &motores[ 0 ] );
    }

    if ( ps3.analog.button.down )
    {
        if ( sp.z - 0.1 > 0 )
            sp.z = sp.z - 0.1;
        motores[ 0 ].decrease_dc( &motores[ 0 ] );
    }
    
    /*
    if (ps3.analog.stick.ry > 0)
    {
        printf("Palanca derecha hacia arriba.");
        move_bw(MCPWM_UNIT_0, MCPWM_TIMER_0);
    }
    if (ps3.analog.stick.ry < 0)
    {
        printf("Palanca derecha hacia abajo.");
        move_fd(MCPWM_UNIT_0, MCPWM_TIMER_0);
    }
    */
}



/*
Nombre: __init__

Descripción: inicializa la dirección MAC del ESP32 con la dirección MAC
             del Joystick. A su vez, establece la conexión entre el Joystick
             y la ESP32.

Devuelve: N/A.
*/
init_error_t joystick_init( struct Joystick *self )
{
    new_mac( &mac );
    ESP_LOGI( TAG, "Inicializando Joystick..." );

    mac.nvs_init( &mac );
    mac.get_mac_address( &mac );
    init_error_t set_mac = mac.set_mac_address( &mac, self->mac );
    if( set_mac != INIT_OK )
        return set_mac;
    ps3SetEventCallback( controller_event_cb );
    ps3SetBluetoothMacAddress( self->mac );
    ps3Init();
    while( !ps3IsConnected() )
    {
        vTaskDelay( pdMS_TO_TICKS( 10 ) );
    }

    ESP_LOGI( TAG, "Joystick inicializado." );
    
    return INIT_OK;
}


void new_joystick( struct Joystick *self, const uint8_t mac_address[ MAC_ADDR_SIZE ] )
{
    /*
    * Atributos
    * ---------
    * 
    * */
    for ( int i = 0; i < MAC_ADDR_SIZE; i++ )
        self->mac[ i ] = mac_address[ i ];
    
    /*
    * Métodos
    * -------
    * 
    * */
    self->joystick_init = joystick_init;
}
