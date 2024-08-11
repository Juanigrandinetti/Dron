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
 ************************************
             PROGRAMA
 ************************************
*/


void app_main( void )
{
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
    * => Actualizar variables de estado.
    * => Actualizar controladores PID.
    * => Actualizar DC de cada motor.
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
        ESP_LOGI( TAG, "Distancia: %.2fcm", dron.z );
        vTaskDelay( pdMS_TO_TICKS( 500 ) );
    }
    
}


void vTaskUpdateDistance( void * pvParameters )
{
    while ( 1 )
    {
        dron.update_distance( &dron );
        vTaskDelay( pdMS_TO_TICKS( 1000 ) );
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
