#include <stdio.h>
#include <spiffs_partition.h>


void restart( void )
{
    for ( int i = 1; i < 4; i++ )
    {
        ESP_LOGE( "FileSystem", "Reiniciando sistema en %ds", i );
        vTaskDelay( pdMS_TO_TICKS( 1000 ) );
    }
    esp_restart();
}


esp_err_t erase_partition( struct Flash* self, const esp_partition_t* partition, size_t starting_position, size_t size_to_erase )
{
    esp_err_t res_erase = esp_partition_erase_range( partition, starting_position, size_to_erase );
    if ( res_erase != ESP_OK )
    {
        ESP_LOGE( "FileSystem", "Error al borrar la partición SPIFFS ( %s )", esp_err_to_name( res_erase ) );
        return res_erase;
    }
    else
        return ESP_OK;
}


const esp_partition_t* get_partition( struct Flash* self )
{
    const esp_partition_t* partition = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA,
        ESP_PARTITION_SUBTYPE_DATA_SPIFFS,
        self->cfg.partition_label);
    if ( partition != NULL )
    {
        ESP_LOGI( "FileSystem", "Dirección: %lu. Tamaño: %lu. Label: %s", partition->address, partition->size, partition->label );
        return partition;
    }
    else
    {
        ESP_LOGE( "FileSystem", "Partición SPIFFS no encontrada." );
        return partition;
    }
}


void spiffs_info( struct Flash* self )
{
    esp_err_t res_info = esp_spiffs_info( self->cfg.partition_label, &self->total, &self->used );
    if ( res_info != ESP_OK )
        ESP_LOGE( "FileSystem", "Error al obtener información de la partición ( %s )", esp_err_to_name( res_info ) );
    else
        ESP_LOGI( "FileSystem", "Tamaño de la partición: %d bytes. En uso: %d bytes", self->total, self->used );
}


void spiffs_cfg( struct Flash* self )
{
    esp_err_t res_register = esp_vfs_spiffs_register( &self->cfg );
    if ( res_register != ESP_OK )
    {
        switch ( res_register )
        {
        case ESP_ERR_NO_MEM:
            ESP_LOGE( "FileSystem", "Memoria insuficiente." );
            break;
        case ESP_ERR_INVALID_STATE:
            ESP_LOGE( "FileSystem", "Partición previamente configurada o encriptada" );
            break;
        case ESP_ERR_NOT_FOUND:
            ESP_LOGE( "FileSystem", "Partición no encontrada." );
            break;
        case ESP_FAIL:
            ESP_LOGE( "FileSystem", "Error al configurar o formatear la partición." );
            break;
        }
        ESP_LOGE( "FileSystem", "Error al inicializar SPIFFS ( %s )", esp_err_to_name( res_register ) );
        restart();
    }
    else
        ESP_LOGI( "FileSystem", "Partición SPIFFS configurada." );
}


void new_spiffs( struct Flash* self )
{
    /*
    * Atributos
    * ---------
    * 
    * */
    self->cfg.base_path = "/storage";
    self->cfg.partition_label = "storage";
    self->cfg.max_files = 5;
    self->cfg.format_if_mount_failed = true;
    self->total = 0;
    self->used = 0;

    /*
    * Métodos
    * -------
    * 
    * */
    self->spiffs_cfg = spiffs_cfg;
    self->spiffs_info = spiffs_info;
    self->get_partition = get_partition;
    self->erase_partition = erase_partition;
}
