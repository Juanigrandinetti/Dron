#include <main.h>


/*
 ************************************
             CONSTANTES
 ************************************
*/


#define STACK_SIZE 1024                     /* Memoria para la ejecución de tareas en paralelo. */
                                            /*
                                            * Máximo de muestras por variable ( 4 estados + 4 setpoints = 8 variables ).
                                            * 
                                            * - Tamaño de la partición SPIFFS: 956561 bytes.
                                            * - Tamaño de cada dato: 251 bytes.
                                            * - Cantidad de muestras por variable = [ ( 956561 /  251 )  / 8 ] = 436 datos.
                                            * - Total de bytes usados = 436 [ datos ] * 251 [ bytes / dato ] * 8 variables = 875488 bytes.
                                            * 
                                            * */
#define SAMPLES 10
#define BUFF_SIZE 1024                      /* Tamaño en bytes de los datos transferidos por UART. */
#define SEND_DATA_CMD "send"                /* Comando para transferir datos de la memoria flash al puerto serie. */
#define BAUD_RATE 115200                    /* Tasa de transferencia de bits/segundo. */


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
static QueueHandle_t uart_queue;                    /* Esta variable se usa para crear un evento para recibir y enviar datos por puerto serie ( UART ). */


/*
 ************************************
             FUNCIONES
 ************************************
*/


/*
* Inicializa todas las tareas.
*
* */
esp_err_t TasksDron( void );

/*
* Almacena datos de los sensores en la partición SPIFFS, de la memoria flash del sistema.
*
* */
void vTaskPrintStates( void * pvParameters );

/*
* Actualiza los estados con los valores que miden los sensores.
*
* */
void vTaskUpdateDistance( void * pvParameters );

/*
* Lee el puerto serie y envía todos los datos que se encuentren en la memoria flash del sistema
* por puerto serie.
*
* */
void vTaskUartEvent( void * pvParameters );

/*
* Configurar e inicializar la comunicación serie por UART.
*
* */
void uart_init( void );


/*
 ************************************
             PROGRAMA
 ************************************
*/


void app_main( void )
{
    uart_init();

    /*
    * Instanciar objetos
    * ------------------
    * 
    * Para instanciar un objeto es necesario llamar a la función new_... de la clase que corresponda
    * ( en este caso la clase es Dron ).
    * 
    * */

    new_dron( &dron );
    new_spiffs( &spiffs );

    /*
    * Configurar la partición SPIFFS para almacenar los estados y setpoints en un archivo .txt.
    *
    * */
    spiffs.spiffs_cfg( &spiffs );
    const esp_partition_t* partition = spiffs.get_partition( &spiffs );
    if ( partition != NULL )
        spiffs.erase_partition( &spiffs, partition, 0x0, partition->size );
    spiffs.spiffs_info( &spiffs );


    /*
    * Inicialización
    * --------------
    *
    *  Inicializar el objeto.
    * */

    if ( dron.init( &dron ) != INIT_OK )
        restart();
    else
        ESP_LOGI( TAG, "Dron inicializado." );

    // btd_init();

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
        ESP_LOGI( TAG, "Tareas inicializadas." );
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

    /*
    xTaskCreate(
        vTaskPrintStates,
        "vTaskPrintStates",
        STACK_SIZE * 3,
        &ucParameterToPass,
        1,
        &xHandle
    );
    */

    xTaskCreate(
        vTaskUartEvent,
        "vTaskUartEvent",
        STACK_SIZE * 3,
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
        /*
        * Trama = <sp_z, z;sp_roll, roll;sp_pitch, pitch;sp_yaw, yaw>
        *
        * */
        // ESP_LOGI( TAG, "%.2f,%.4f", sp.z, dron.z );
        if ( cont < SAMPLES )
        {
            fp = fopen( file, "a" );
            if ( fp != NULL )
            {
                fprintf( fp, "%.2f,%.4f\n", sp.z, dron.z );
                printf( "Variables almacenadas en la memoria flash del sistema: %d\n", cont + 1 );
                cont++;
            }
            else
                ESP_LOGE( "FileSystem", "Error al escribir el archivo .txt de la partición SPIFFS." );
            fclose( fp );
        }
        else
        {
            ESP_LOGI( "FileSystem", "Todos los datos han sido almacenados en la memoria flash del sistema." );
            ESP_LOGI( "FileSystem", "Eliminando tarea..." );
            vTaskDelete( NULL );
        }
        vTaskDelay( pdMS_TO_TICKS( 500 ) );
    }
}


void vTaskUpdateDistance( void * pvParameters )
{
    while ( 1 )
    {
        dron.update_distance( &dron );
        vTaskDelay( pdMS_TO_TICKS( 500 ) );
    }
    
}


void vTaskUartEvent( void * pvParameters )
{
    const char* res = "OK\n";
    char stored_data[ 256 ];
    uart_event_t event;
    uint8_t* rx_data = ( uint8_t* ) malloc( BUFF_SIZE );
    while( 1 )
    {
        if ( xQueueReceive( uart_queue, ( void* ) &event, ( TickType_t ) portMAX_DELAY ) )
        {
            bzero( rx_data, BUFF_SIZE );
            switch ( event.type )
            {
            case UART_DATA:
                uart_read_bytes( UART_NUM_0, rx_data, event.size, portMAX_DELAY );
                if ( strcmp( ( const char* ) rx_data, SEND_DATA_CMD ) == 0 )
                {
                    uart_write_bytes( UART_NUM_0, res, sizeof( res ) );
                    printf( "Comenzando a enviar datos al puerto serie en... \n" );
                    for (int i = 1; i < 4; i++)
                    {
                        printf( "%d segundos\n", i );
                        vTaskDelay( pdMS_TO_TICKS( 1000 ) );
                    }
                    fp = fopen( file, "r" );
                    if ( fp != NULL )
                    {
                        while( fgets( stored_data, sizeof( stored_data ), fp ) != NULL )
                            printf( "%s", stored_data );
                    }
                    else
                        ESP_LOGE( "FileSystem", "Error al leer el archivo .txt de la partición SPIFFS, o bien el mismo se encuentra vacío." );
                    fclose( fp );
                }
                break;
            default:
                break;
            }
        }
    }
    free( rx_data );
    rx_data = NULL;
    vTaskDelete( NULL );
}


void uart_init( void )
{
    uart_config_t uart_cfg = {
        .baud_rate = BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    ESP_ERROR_CHECK( uart_driver_install( UART_NUM_0, BUFF_SIZE * 2, BUFF_SIZE * 2, 20, &uart_queue, 0 ) );
    ESP_ERROR_CHECK( uart_param_config( UART_NUM_0, &uart_cfg ) );
    ESP_ERROR_CHECK( uart_set_pin( UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE ) );
}
