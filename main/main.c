#include <main.h>


/*
 ************************************
             CONSTANTES
 ************************************
*/


#define STACK_SIZE 1024                     /* Memoria para la ejecución de tareas en paralelo. */


/*
 ************************************
         VARIABLES GLOBALES
 ************************************
*/


Flash spiffs;
Dron dron;                                          /* Esta variable se usa para crear una instancia de un objeto de clase Dron. */
                                                    /*
                                                    * Esta variable se usa para abrir un archivo .txt ( r/w ) dentro de la partición SPIFFS,
                                                    * para almacenar los datos de vuelo.
                                                    * 
                                                    * */
FILE* fp = NULL;
const char* file = "/storage/valores_medidos.txt";  /* Nombre del archivo .txt utilizado en la partición SPIFFS. */
int cont = 0;
const esp_partition_t* partition;                   /* Puntero a la partición SPIFFS. */


/*
 ************************************
             FUNCIONES
 ************************************
*/


/* Inicializa la partición SPIFFS. */
void spiffs_init( void );


/* Inicializa todas las tareas. */
esp_err_t TasksDron( void );


/* Actualiza los estados con los valores que miden los sensores. */
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
    * 
    * */

    new_dron( &dron );
    new_spiffs( &spiffs );

    /*
    * Inicialización
    * --------------
    *
    *  Inicializar los objetos.
    * 
    * */

    spiffs_init();

    if ( dron.init( &dron ) != INIT_OK )
        restart();
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
    {
        ESP_LOGE( TAG, "Error al inicializar tareas. ( %s )", esp_err_to_name( res ) );
        restart();
    }
    else
    {
        ESP_LOGI( TAG, "Tareas inicializadas." );
        gpio_set_level( 2, 0 );
        vTaskDelay( pdMS_TO_TICKS( 500 ) );
        gpio_set_level( 2, 1 );
    }
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
        STACK_SIZE * 2,
        &ucParameterToPass,
        1,
        &xHandle
    );

    return ESP_OK;
}


void vTaskUpdateDistance( void * pvParameters )
{
    while ( 1 )
    {
        dron.update_distance( &dron );
        vTaskDelay( pdMS_TO_TICKS( 500 ) );
    }
    
}


void spiffs_init( void )
{
    spiffs.spiffs_cfg( &spiffs );
    partition = spiffs.get_partition( &spiffs );
    if ( partition != NULL )
        spiffs.erase_partition( &spiffs, partition, 0x0, partition->size );
    spiffs.spiffs_info( &spiffs );
}
