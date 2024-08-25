#include <stdio.h>
#include <hcsr04.h>


static portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
#define PORT_ENTER_CRITICAL portENTER_CRITICAL( &mux )
#define PORT_EXIT_CRITICAL portEXIT_CRITICAL( &mux )


void gpio_init( int pin, gpio_mode_t mode )
{
    gpio_reset_pin( pin );
    gpio_set_direction( pin, mode );
}


init_error_t hcsr04_init( struct Hcsr04* self )
{
    ESP_LOGI( TAG, "Inicializando HC-SR04..." );
    
    gpio_init( self->trig, GPIO_MODE_OUTPUT );
    gpio_init( self->echo, GPIO_MODE_INPUT );

    ESP_LOGI( TAG, "HC-SR04 inicializado." );
    return INIT_OK;
}


esp_err_t measure_Distance( int64_t *time, int trig_pin, int echo_pin )
{
    /* IMPORTANTE!!
    *
    * No agregar prints ni logs en la sección contenida dentro de PORT_ENTER_CRITICAL
    * y PORT_EXIT_CRITICAL ya que va a generar un error.
    * 
    * Tampoco se deben eliminar ya que son necesarios para una medición precisa.
    * 
    * No agregar interrupciones.
    * 
    * */

    PORT_ENTER_CRITICAL;

    gpio_set_level( trig_pin, 0 );   // Setear el pin TRIGGER en BAJO.
    ets_delay_us( LOW_TRIG_DELAY );  // Delay 4us.
    gpio_set_level( trig_pin, 1 );   // Setear el pin TRIGGER en ALTO.
    ets_delay_us( HIGH_TRIG_DELAY ); // Delay 10us.
    gpio_set_level( trig_pin, 0 );   // Setear el pin TRIGGER en BAJO => onda ultrasonica de 40KHz enviada y el pin ECHO en ALTO.

    if( gpio_get_level( echo_pin ) )   // Chequeamos que ECHO no haya quedado en 1 de la onda ultrasónica anterior.
        return ESP_ERR_INVALID_STATE;

    int64_t start = esp_timer_get_time();
    while( !gpio_get_level( echo_pin ) ) // Chequeamos que ECHO valga 1, si vale 0 durante 6000 us retornamos timeout.
    {
        if( esp_timer_get_time() - start > ECHO_TIMEOUT )
            return ESP_ERR_TIMEOUT;
    }

    int64_t echo_start = esp_timer_get_time();
    int64_t echo_end = echo_start;

    while( gpio_get_level( echo_pin ) ) // Calculamos cuánto dura el pulso de ECHO.
        echo_end = esp_timer_get_time();

    PORT_EXIT_CRITICAL;

    *time = echo_end - echo_start;
    
    return ESP_OK;
}

float get_distance( struct Hcsr04* self )
{
    int64_t time;

    esp_err_t res = measure_Distance( &time, self->trig, self->echo );
        if( res != ESP_OK )
        {
            switch( res )
            {
                case ESP_ERR_INVALID_STATE:
                    ESP_LOGE( TAG, "pin ECHO todavía en ALTO." );
                    break;
                case ESP_ERR_TIMEOUT:
                    ESP_LOGE( TAG, "Error al setear el pin ECHO." );
                    break;
                default:
                    printf( "%s\n", esp_err_to_name( res ) );
            }
            return -1;
        }
        else
            return ( ( 1.0 * time ) * ( V_SONIDO / 2 ) / 100 );
}


void new_hcsr04( Hcsr04* self, int trig, int echo )
{
    /*
    * Atributos
    * ---------
    * 
    * */
    self->trig      = trig;
    self->echo      = echo;
    self->distancia = 0;

    /*
    * Métodos
    * -------
    * 
    * */
    self->init         = hcsr04_init;
    self->get_distance = get_distance;
}
