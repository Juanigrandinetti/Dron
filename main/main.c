#include <stdio.h>
#include <dron.h>


/*
 ************************************
             CONSTANTES
 ************************************
*/


#define STACK_SIZE 1024 * 2 // Memoria para la ejecución de tareas en paralelo.


/*
 ************************************
         VARIABLES GLOBALES
 ************************************
*/

SemaphoreHandle_t GlobalKey = 0;
Dron dron; // Esta variable se usa para crear una instancia de un objeto de clase Dron.


/*
 ************************************
             FUNCIONES
 ************************************
*/

esp_err_t TasksDron( void );
void vTaskPrintStates( void * pvParameters );
void vTaskUpdateDistance( void * pvParameters );

/*
esp_err_t move_task( void );
void vTaskMoveDron( void * pvParameters );
*/


/*
 ************************************
             PROGRAMA
 ************************************
*/


void app_main( void )
{
    GlobalKey = xSemaphoreCreateBinary();

    /*
    * Instanciar objetos
    * ------------------
    * 
    * Para instanciar un objeto es necesario llamar a la función new_... de la clase que corresponda
    * ( en este caso la clase es Dron ).
    * 
    * */

    new_dron( &dron );

    /*
    * Inicialización
    * --------------
    *
    *  Inicializar el objeto.
    * */

    if ( dron.init( &dron ) != INIT_OK )
        esp_restart();
    else
        ESP_LOGI( TAG, "Dron inicializado." );

    /*
    * Tareas ( RTOS ) 
    * ---------------
    * 
    * Ejecutar tareas asociadas a
    * 
    * => Actualizar los estados del dron.
    * => Actualizar los controladores PID.
    * => Actualizar el DC de cada motor.
    * 
    * */
    esp_err_t res = TasksDron();
    if ( res != ESP_OK )
        ESP_LOGE( TAG, "%s", esp_err_to_name( res ) );
}


/*
 ************************************
                TAREAS
 ************************************
*/


esp_err_t TasksDron( void )
{
    static uint8_t ucParameterToPass;
    TaskHandle_t xHandle = NULL;

    xTaskCreate(
        vTaskUpdateDistance,
        "vTaskUpdateDistance",
        STACK_SIZE,
        &ucParameterToPass,
        1,
        &xHandle
    );

    xTaskCreate(
        vTaskPrintStates,
        "vTaskPrintStates",
        STACK_SIZE,
        &ucParameterToPass,
        1,
        &xHandle
    );

    return ESP_OK;
}


void vTaskPrintStates( void * pvParameters )
{
    while ( 1 )
    {
        if ( xSemaphoreTake( GlobalKey, portMAX_DELAY ) )
        {
            printf( "Llave recibida, iniciando Task print.\n" );
            ESP_LOGI( TAG, "Distancia: %.2fcm", dron.z );
        }
        printf( "Task print hivernando.\n" );
        vTaskDelay( pdMS_TO_TICKS( 500 ) );
    }
    
}


void vTaskUpdateDistance( void * pvParameters )
{
    while ( 1 )
    {
        printf( "Iniciando Task datos.\n" );
        dron.update_distance( &dron );
        xSemaphoreGive( GlobalKey );
        vTaskDelay( pdMS_TO_TICKS( 1000 ) );
        printf( "Entregando llave.\n" );
    }
    
}


/*
void vTaskUpdateAngles( void * pvParameters )
{
    while( 1 )
    {
        dron.update_angles( &dron );
        vTaskDelay( pdMS_TO_TICKS( 1000 ) );
    }
}


void vTaskUpdatedc( void * pvParameters )
{
    while( 1 )
    {
        dron.updatedc( &dron );
        vTaskDelay( pdMS_TO_TICKS( 500 ) );
    }
}
*/
