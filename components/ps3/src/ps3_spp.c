#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "include/ps3.h"
#include "include/ps3_int.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <freertos/semphr.h>
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "esp_bt_device.h"
#include "esp_spp_api.h"


#define PS3_TAG         "PS3_SPP"
#define PS3_DEVICE_NAME "PS3 Host"
#define PS3_SERVER_NAME "PS3_SERVER"
#define TOTAL_SAMPLES   10
#define TASK_STACK      1024
#define SIZE_TRAMA      251

extern char* file;
extern int cont;
extern FILE* fp;
extern Dron dron;
extern Flash spiffs;
extern Pid pid[ 4 ];
extern const esp_partition_t* partition;
TaskHandle_t xLoadToFlashHandle = NULL;
bt_pid bt_pid_s;

/********************************************************************************/
/*              L O C A L    F U N C T I O N     P R O T O T Y P E S            */
/********************************************************************************/
static void ps3_spp_callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param);
void parse_btcmd( char* data );
void vTaskLoadToFlash( void * pvParameters );
void vTaskUpdatePIDParameters( void * pvParameters );
void vTaskExctractFromFlash( void * pvParameters );
void vTaskEraseFlash( void * pvParameters );


/********************************************************************************/
/*                      P U B L I C    F U N C T I O N S                        */
/********************************************************************************/

/*******************************************************************************
**
** Function         ps3_spp_init
**
** Description      Initialise the SPP server to allow to be connected to
**
** Returns          void
**
*******************************************************************************/
void ps3_spp_init()
{
    esp_err_t ret;


    ESP_ERROR_CHECK( esp_bt_controller_mem_release( ESP_BT_MODE_BLE ) );

#ifndef ARDUINO_ARCH_ESP32
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if ((ret = esp_bt_controller_init(&bt_cfg)) != ESP_OK) {
        ESP_LOGE(PS3_TAG, "%s initialize controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bt_controller_enable(BT_MODE)) != ESP_OK) {
        ESP_LOGE(PS3_TAG, "%s enable controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bluedroid_init()) != ESP_OK) {
        ESP_LOGE(PS3_TAG, "%s initialize bluedroid failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bluedroid_enable()) != ESP_OK) {
        ESP_LOGE(PS3_TAG, "%s enable bluedroid failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }
#endif

    if ((ret = esp_spp_register_callback(ps3_spp_callback)) != ESP_OK) {
        ESP_LOGE(PS3_TAG, "%s spp register failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    esp_spp_cfg_t bt_spp_cfg = {
        .mode = ESP_SPP_MODE_CB,
        .enable_l2cap_ertm = true,
        .tx_buffer_size = 0, /* Only used for ESP_SPP_MODE_VFS mode */
    };
    if ((ret = esp_spp_enhanced_init(&bt_spp_cfg)) != ESP_OK) {
        ESP_LOGE(PS3_TAG, "%s spp init failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }
}


/*******************************************************************************
**
** Function         ps3_spp_deinit
**
** Description      Deinitialise the SPP server
**
** Returns          void
**
*******************************************************************************/
void ps3_spp_deinit()
{

    esp_err_t ret;

    if ((ret = esp_spp_deinit()) != ESP_OK) {
        ESP_LOGE(PS3_TAG, "%s spp deinit failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

#ifndef ARDUINO_ARCH_ESP32
    if ((ret = esp_bluedroid_disable()) != ESP_OK) {
        ESP_LOGE(PS3_TAG, "%s disable bluedroid failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bluedroid_deinit()) != ESP_OK) {
        ESP_LOGE(PS3_TAG, "%s deinitialize bluedroid failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bt_controller_disable()) != ESP_OK) {
        ESP_LOGE(PS3_TAG, "%s disable controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    if ((ret = esp_bt_controller_deinit()) != ESP_OK) {
        ESP_LOGE(PS3_TAG, "%s deinitialize controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }
#endif
}



/********************************************************************************/
/*                      L O C A L    F U N C T I O N S                          */
/********************************************************************************/

/*******************************************************************************
**
** Function         ps3_spp_callback
**
** Description      Callback for SPP events, only used for the init event to
**                  configure the SPP server
**
** Returns          void
**
*******************************************************************************/
static void ps3_spp_callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
    switch ( event )
    {
    case ESP_SPP_INIT_EVT:
        if ( param->init.status == ESP_SPP_SUCCESS )
        {
            ESP_LOGI( PS3_TAG, "ESP_SPP_INIT_EVT" );
            esp_spp_start_srv( ESP_SPP_SEC_NONE, ESP_SPP_ROLE_SLAVE, 0, PS3_SERVER_NAME );
        }
        break;
    case ESP_SPP_START_EVT:
        if ( param->start.status == ESP_SPP_SUCCESS )
        {
            ESP_LOGI( PS3_TAG, "ESP_SPP_START_EVT" );
            esp_bt_dev_set_device_name( PS3_DEVICE_NAME );
            esp_bt_gap_set_scan_mode( ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE );
        }
        break;
    case ESP_SPP_SRV_OPEN_EVT:
        ESP_LOGI( PS3_TAG, "Un nuevo dispositivo se ha conectado, Dirección MAC: %02x:%02x:%02x:%02x:%02x:%02x",
        param->srv_open.rem_bda[ 0 ], param->srv_open.rem_bda[ 1 ], param->srv_open.rem_bda[ 2 ],
        param->srv_open.rem_bda[ 3 ], param->srv_open.rem_bda[ 4 ], param->srv_open.rem_bda[ 5 ] );
        break;
    case ESP_SPP_CLOSE_EVT:
        ESP_LOGI( PS3_TAG, "Un dispositivo se ha desconectado, Dirección MAC: %02x:%02x:%02x:%02x:%02x:%02x",
        param->srv_open.rem_bda[ 0 ], param->srv_open.rem_bda[ 1 ], param->srv_open.rem_bda[ 2 ],
        param->srv_open.rem_bda[ 3 ], param->srv_open.rem_bda[ 4 ], param->srv_open.rem_bda[ 5 ] );
        break;
    case ESP_SPP_DATA_IND_EVT:
        esp_spp_write( param->data_ind.handle, param->data_ind.len, param->data_ind.data ); /* Es un echo, transmite exactamente lo mismo que recibió. */
        parse_btcmd( ( char* ) param->data_ind.data );
        break;
    default:
        break;
    }
}


void parse_btcmd( char* data )
{
    char* token;
    char* rest = data;
                                                    /* Trama PID: <Etiqueta,Variable,Tipo,Valor>
                                                    *
                                                    * - Etiqueta: pid.
                                                    * - Variable: z, roll, pitch, yaw.
                                                    * - Tipo: P, I, D
                                                    * - Valor: ejemplo: 10
                                                    * Ejemplo de trama: <pid,z,p,10>
                                                    * 
                                                    * */
    char* tokens[ 4 ] = { "", "", "", "" };
    int i = 0;
    while ( ( token = strtok_r( rest, ",", &rest ) ) )
    {
        tokens[i] = token;
        i++;
    }

    if ( strcmp( tokens[ 0 ], "pid" ) == 0 )
    {
        bt_pid_s.pid_type = tokens[ 2 ];
        bt_pid_s.value    = tokens[ 3 ];

        if ( strcmp( tokens[ 1 ], "z" ) == 0 )
            bt_pid_s.estado = 0;
        else
        {
            if ( strcmp( tokens[ 1 ], "roll" ) == 0 )
                bt_pid_s.estado = 1;
            else
            {
                if ( strcmp( tokens[ 1 ], "pitch" ) == 0 )
                    bt_pid_s.estado = 2;
                else
                {
                    if ( strcmp( tokens[ 1 ], "yaw" ) == 0 )
                        bt_pid_s.estado = 3;
                    else
                        bt_pid_s.estado = -1;
                }
            }
        }
        xTaskCreate( vTaskUpdatePIDParameters, "vTaskUpdatePIDParameters", TASK_STACK * 2, NULL, 1, NULL );
    }
    else
    {
        if ( strcmp( tokens[ 0 ], "start" ) == 0 )
            xTaskCreate( vTaskLoadToFlash, "vTaskLoadToFlash", TASK_STACK * 2, NULL, 1, &xLoadToFlashHandle );
        else
        {
            if ( strcmp( tokens[ 0 ], "stop" ) == 0 )
            {
                if ( ( fp = fopen( file, "r" ) != NULL ) )
                {
                    vTaskDelete( xLoadToFlashHandle );
                    printf( "Tarea pausada.\n" );
                }
                else
                    ESP_LOGE( "FileSystem", "Archivo vacío." );
            }
            else
            {
                if ( strcmp( tokens[ 0 ], "send" ) == 0 )
                    xTaskCreate( vTaskExctractFromFlash, "vTaskExctractFromFlash", TASK_STACK * 2, NULL, 1, NULL );
                else
                {
                    if ( strcmp( tokens[ 0 ], "erase" ) == 0 )
                        xTaskCreate( vTaskEraseFlash, "vTaskEraseFlash", TASK_STACK * 2, NULL, 1, NULL );
                }
            }
        }
    }
}


void vTaskExctractFromFlash( void * pvParameters )
{
    char stored_data[ 256 ];

    while ( 1 )
    {
        fp = fopen( file, "r" );
        if ( fp != NULL )
        {
            while ( fgets( stored_data, sizeof( stored_data ), fp ) )
                printf( "%s", stored_data );
            fclose( fp );
            vTaskDelete( NULL );
        }
        else
            ESP_LOGE( "FileSystem", "Error al leer el archivo .txt de la partición SPIFFS, o bien el mismo se encuentra vacío." );
        fclose( fp );
        vTaskDelete( NULL );
    }
}


void vTaskLoadToFlash( void * pvParameters )
{
    while ( spiffs.total - spiffs.used > SIZE_TRAMA )
    {
        spiffs.spiffs_info( &spiffs );
        fp = fopen( file, "a" );
        if ( fp != NULL )
        {
            fprintf( fp, "%.2f, %.4f\n", sp.z, dron.z );
            printf( "Variables almacenadas en la memoria flash del sistema: %d\n", cont + 1 );
            cont++;
        }
        else
        {
            ESP_LOGE( "FileSystem", "Error al escribir el archivo .txt de la partición SPIFFS." );
            break;
        }
        fclose( fp );
        vTaskDelay( pdMS_TO_TICKS( 500 ) );
    }
    ESP_LOGW( "FileSystem", "Espacio insuficiente." );
    vTaskDelete( NULL );
}


void vTaskUpdatePIDParameters( void * pvParameters )
{
    while ( 1 )
    {
        if ( strcmp( bt_pid_s.pid_type, "p" ) == 0 ) 
        {
            pid[ bt_pid_s.estado ].kp = atof( bt_pid_s.value );
            switch ( bt_pid_s.estado )
            {
                case 0:
                    ESP_LOGI( TAG, "{ z } Kp: %.2f", pid[ bt_pid_s.estado ].kp );    
                    break;
                case 1:
                    ESP_LOGI( TAG, "{ Roll } Kp: %.2f", pid[ bt_pid_s.estado ].kp );    
                    break;
                case 2:
                    ESP_LOGI( TAG, "{ Pitch } Kp: %.2f", pid[ bt_pid_s.estado ].kp );    
                    break;
                case 3:
                    ESP_LOGI( TAG, "{ Yaw } Kp: %.2f", pid[ bt_pid_s.estado ].kp );
                    break;
                default:
                    break;
            }
        }
        else
        {
            if ( strcmp( bt_pid_s.pid_type, "i" ) == 0 )
            {
                pid[ bt_pid_s.estado ].ki = atof( bt_pid_s.value );
                switch ( bt_pid_s.estado )
                {
                    case 0:
                        ESP_LOGI( TAG, "{ z } Ki: %.2f", pid[ bt_pid_s.estado ].ki );    
                        break;
                    case 1:
                        ESP_LOGI( TAG, "{ Roll } Ki: %.2f", pid[ bt_pid_s.estado ].ki );    
                        break;
                    case 2:
                        ESP_LOGI( TAG, "{ Pitch } Ki: %.2f", pid[ bt_pid_s.estado ].ki );    
                        break;
                    case 3:
                        ESP_LOGI( TAG, "{ Yaw } Ki: %.2f", pid[ bt_pid_s.estado ].ki );
                        break;
                    default:
                        break;
                }
            }
            else
            {
                if ( strcmp( bt_pid_s.pid_type, "d" ) == 0 )
                {
                    pid[ bt_pid_s.estado ].kd = atof( bt_pid_s.value );
                    switch ( bt_pid_s.estado )
                    {
                        case 0:
                            ESP_LOGI( TAG, "{ z } Kd: %.2f", pid[ bt_pid_s.estado ].kd ); 
                            break;
                        case 1:
                            ESP_LOGI( TAG, "{ Roll } Kd: %.2f", pid[ bt_pid_s.estado ].kd );
                            break;
                        case 2:
                            ESP_LOGI( TAG, "{ Pitch } Kd: %.2f", pid[ bt_pid_s.estado ].kd );
                            break;
                        case 3:
                            ESP_LOGI( TAG, "{ Yaw } Kd: %.2f", pid[ bt_pid_s.estado ].kd );
                            break;
                        default:
                            break;
                    }
                }
            }
        }
        vTaskDelete( NULL );
    }
}


void vTaskEraseFlash( void * pvParameters )
{
    while ( 1 )
    {
        if ( remove( file ) == 0 )
        {
            ESP_LOGI( "FileSystem", "Datos borrados." );
            cont = 0;
        }
        else
            ESP_LOGE( "FileSystem", "Error al borrar los datos." );
        vTaskDelete( NULL );
    }
}
