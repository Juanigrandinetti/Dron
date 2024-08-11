#include <stdio.h>
#include <change_mac.h>


/*
Nombre: get_mac_address

Descripción: Obtiene la dirección MAC de la placa ESP32.

Devuelve: N/A.
*/
void get_mac_address( struct Mac* self )
{
    uint8_t mac[ MAC_ADDR_SIZE ];
    esp_wifi_get_mac( ESP_IF_WIFI_STA, mac );
    ESP_LOGI( TAG, "Dirección MAC original: %02x:%02x:%02x:%02x:%02x:%02x\n",
        mac[ 0 ], mac[ 1 ], mac[ 2 ], mac[ 3 ], mac[ 4 ], mac[ 5 ] );
}

/*
Nombre: set_mac_address

Descripción: Modifica la dirección MAC de la placa ESP32.

Devuelve: 1 si se logró modificar la dirección MAC, caso contrario 0.
*/
init_error_t set_mac_address( struct Mac* self, uint8_t *mac )
{
    esp_err_t err = esp_wifi_set_mac( ESP_IF_WIFI_STA, mac );
    if ( err == ESP_OK )
    {
        fflush( stdout );
        ESP_LOGI( TAG, "Nueva dirección MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
            mac[ 0 ], mac[ 1 ], mac[ 2 ], mac[ 3 ], mac[ 4 ], mac[ 5 ] );

        return INIT_OK;
    }
    else
        return JS_MAC_ERROR;
}

/*
Nombre: nvs_ini

Descripción: inicializa la memoria flash no volatil (NVS) del sistema.

Devuelve: N/A.
*/
void nvs_ini( struct Mac* self )
{
    // Inicializar la memoria flash no volatil (NVS) del sistema 
    esp_err_t ret = nvs_flash_init();
	if ( ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND )
	{
		ESP_ERROR_CHECK( nvs_flash_erase() );
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK( ret );

    ESP_ERROR_CHECK( esp_netif_init() );
    ESP_ERROR_CHECK( esp_event_loop_create_default() );
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_mode( WIFI_MODE_STA ) );
    ESP_ERROR_CHECK( esp_wifi_start() );
}


void new_mac( Mac* self )
{
    self->nvs_init = nvs_ini;
    self->get_mac_address = get_mac_address;
    self->set_mac_address = set_mac_address;
}
