
#include "hvac.h"
#include "atheros_driver_includes.h"
#include "atheros_wifi_api.h"
#include "atheros_main.h"
#include "WiFi_GT202.h"
#include "string.h"

#if (ENABLE_STACK_OFFLOAD == 1)
    #error This demo requires ENABLE_STACK_OFFLOAD = 0 in a_config.h.  Rebuild BSP after changing
#endif

#if (MQX_ENABLE_HSRUN)
    #if (BSP_TWR_K22F120M || BSP_TWR_K22F120M256R || BSP_FRDM_K22F120M)
        #warning To use the flashx driver to write the Wi-Fi parameters to flash, MQX_ENABLE_HSRUN must be 0 in user_config.h.  Rebuild BSP after changing. 
    #endif
#endif

const char *     CipherName[]   = {"TKIP", "CCMP"};
LWEVENT_STRUCT   lwevent_wifi_connect;
WIFI_PARAMS_PTR   gp_WIFI_Params;

#define LWEVENT_WIFI_CONNECTED        1

MQX_FILE_PTR    flash_file;

// Function Prototypes
extern int32_t  clear_wep_keys();
static uint32_t set_cipher (uint32_t dev_num, char * u_cipher, char * m_cipher);
void            wait_connect(WIFI_PARAMS_PTR params);
_mqx_int        disconnect_wifi(void);
void            wifi_flash_read(WIFI_PARAMS_PTR params);
        
        
/*FUNCTION*-------------------------------------------------------------
*
* Function Name   : set_cipher()
* Returned Value  : SHELL_EXIT_SUCCESS if ESSID set successfully else ERROR CODE
* Comments        : Sets the Unicast and multicast cipher
*
*END*-----------------------------------------------------------------*/
static uint32_t set_cipher
    (
        uint32_t dev_num,
        char * u_cipher, char * m_cipher
    )
{

    ATH_IOCTL_PARAM_STRUCT  inout_param;
    uint32_t error;
    bool dev_status;
    cipher_t cipher;


     /**************** ucipher **********/

    if(strcmp(u_cipher,"TKIP")==0)
    {
        cipher.ucipher = 0x04;
    }
    else if(strcmp(u_cipher,"CCMP")==0)
    {
        cipher.ucipher = 0x08;
    }
    else
    {
        printf("Invalid ucipher\n");
        return SHELL_EXIT_ERROR;
    }


    /*********** mcipher ************/

    if(strcmp(m_cipher,"TKIP")==0)
    {
        cipher.mcipher = 0x04;
    }
    else if(strcmp(m_cipher,"CCMP")==0)
    {
        cipher.mcipher = 0x08;
    }
    else
    {
        printf("Invalid mcipher\n");
        return SHELL_EXIT_ERROR;
    }


    inout_param.cmd_id = ATH_SET_CIPHER;
    inout_param.data = &cipher;
    inout_param.length = 8;
    error = ENET_mediactl (g_Enet_handle,ENET_MEDIACTL_IS_INITIALIZED,&dev_status);
    if (ENET_OK != error)
    {
        return error;
    }
    if (dev_status == FALSE)
    {
        return ENET_ERROR;
    }

    error = ENET_mediactl (g_Enet_handle,ENET_MEDIACTL_VENDOR_SPECIFIC,&inout_param);
    if (ENET_OK != error)
    {
        return error;
    }
    return SHELL_EXIT_SUCCESS;
}

/*FUNCTION*-----------------------------------------------------------------
*
* Function Name  : wifi_Callback
* Returned Value : N/A
* Comments       : Called from driver on a WiFI connection event
*
*END------------------------------------------------------------------*/
void wifi_Callback(int val)
{

	if(val == A_TRUE)
	{
            wifi_set_property(gp_WIFI_Params, WIFI_WIFI_CONNECTED, TRUE);          
            printf("Connected to %s\n", gp_WIFI_Params->ssid);
            
            #if DEMOCFG_ENABLE_DEVICECLOUD_IO
                  // Check if DCIO needs to be reconnected manually
                 if(wifi_dcio_past_status == eDevConnected)
                 {
                     // DCIO was connected in past, now Wi-Fi just connected
                   
                     // Check if DCIO thinks it is still connect
                     if(dcio_get_connection_status() == eDevConnected)
                        // DCIO thinks it's still connected
                        //dcio_reconnect();       // manually force DCIO to reconnect
                     
                     wifi_dcio_past_status = eDevDisconnected;
                 }
            #endif
                 
            _lwevent_set(&lwevent_wifi_connect, LWEVENT_WIFI_CONNECTED);            
	}
        else if(val == A_FALSE)
        {
            #if DEMOCFG_ENABLE_DEVICECLOUD_IO
                 if(dcio_get_connection_status() == eDevConnected)
                    // DCIO was connected, now lost Wi-Fi connection.  Remember for when Wi-Fi restored
                    wifi_dcio_past_status = eDevConnected;
            #endif
                 
            wifi_set_property(gp_WIFI_Params, WIFI_WIFI_CONNECTED, FALSE);          
        }
}

A_INT32 set_wifi_callback()
{
    ATH_IOCTL_PARAM_STRUCT  inout_param;
    A_INT32 error;
    bool dev_status;

    if (_lwevent_create(&lwevent_wifi_connect, LWEVENT_AUTO_CLEAR) != MQX_OK) {
        printf("\nCreate lwevent_wifi_connect failed");
        _task_block();
    }
    
    inout_param.cmd_id = ATH_SET_CONNECT_STATE_CALLBACK;
    inout_param.data = (void*)wifi_Callback;
    inout_param.length = 4;
    error = ENET_mediactl (g_Enet_handle,ENET_MEDIACTL_IS_INITIALIZED,&dev_status);
    if (ENET_OK != error)
    {
        return error;
    }
    if (dev_status == FALSE)
    {
        return ENET_ERROR;
    }

    error = ENET_mediactl (g_Enet_handle,ENET_MEDIACTL_VENDOR_SPECIFIC,&inout_param);
    if (ENET_OK != error)
    {
        return error;
    }

    return error;
}

void toggle_cipher(WIFI_PARAMS_PTR params)
{
      // Toggle cipher
      if(params->cipher == TKIP)
          params->cipher = CCMP;
      else 
          params->cipher = TKIP;

      set_cipher(DEMOCFG_DEFAULT_DEVICE,(char *)CipherName[params->cipher],(char *)CipherName[params->cipher]);
}

void wait_connect(WIFI_PARAMS_PTR params)
{

  _mqx_uint result;
  
  result = _lwevent_wait_ticks(&lwevent_wifi_connect, LWEVENT_WIFI_CONNECTED, FALSE, 2000);      // Wait 10 seconds to connect
    
  if(result == LWEVENT_WAIT_TIMEOUT)
  {
      // Timed out before connecting
      if((strcmp(params->sec,"wpa2") == 0) || (strcmp(params->sec,"wpa") == 0))
      {
          toggle_cipher(params);
          printf("Attempting to connect with these Wi-Fi parameters:\n");
          wifi_param_print(params);
          iwcfg_commit (DEMOCFG_DEFAULT_DEVICE); 
          
          result = _lwevent_wait_ticks(&lwevent_wifi_connect, LWEVENT_WIFI_CONNECTED, FALSE, 2000);      // Wait 10 seconds to connect
          if(result == LWEVENT_WAIT_TIMEOUT)
          {
              printf("Failed connecting to %s\n", params->ssid);
              if((strcmp(params->sec,"wpa2") == 0) || (strcmp(params->sec,"wpa") == 0))
                  toggle_cipher(params);
          }
      } else
      {
          printf("Failed connecting to %s\n", params->ssid);
      }
  }
}

_mqx_uint wifi_params_init(void)
{
    _mqx_uint   result = MQX_OK, size;
    
    /* Open the flash device */
    flash_file = fopen(FLASH_NAME, NULL);
    if (flash_file == NULL) {
        printf("\nUnable to open flash file %s for Wi-Fi parameters", FLASH_NAME);
        _task_block();
    } else {
        printf("\nFlash file %s opened for Wi-Fi Parameters", FLASH_NAME);
    }

    /* Disable sector cache */
    ioctl(flash_file, FLASH_IOCTL_DISABLE_SECTOR_CACHE, NULL);
    
    // Allocate Memory for parameters
    gp_WIFI_Params = _mem_alloc_zero(sizeof(WIFI_PARAMS));
    if(gp_WIFI_Params == 0)
        return MQX_OUT_OF_MEMORY;
    
    // Read params stored in flash
    wifi_flash_read(gp_WIFI_Params);
    
    
    // Check if Wi-Fi params are valied, or assign default params
    if(gp_WIFI_Params->ssid[0] == 0xFF)
        // First char is erased, use default
        strcpy(gp_WIFI_Params->ssid, DEMOCFG_SSID);
    
    if(gp_WIFI_Params->mode[0] == 0xFF)
        // First char is erased, use default
        strcpy(gp_WIFI_Params->mode, DEMOCFG_NW_MODE);
    
    if(gp_WIFI_Params->sec[0] == 0xFF)
        // First char is erased, use default
        strcpy(gp_WIFI_Params->sec, DEMOCFG_SECURITY);
    
    if(gp_WIFI_Params->passphrase[0] == 0xFF)
        // First char is erased, use default
        strcpy(gp_WIFI_Params->passphrase, DEMOCFG_PASSPHRASE);
    
    if(gp_WIFI_Params->cipher == 0xFF)
        // cipher is erased, use default
        gp_WIFI_Params->cipher = DEMOCFG_DEFAULT_CIPHER;
    
    if(gp_WIFI_Params->wepkey_index == 0xFF)
        // wepkey index is erased, use default
        gp_WIFI_Params->wepkey_index = DEMOCFG_WEP_KEY_INDEX;
    
    wifi_set_property(gp_WIFI_Params, WIFI_WIFI_CONNECTED, FALSE);
    
    //If using WEP, check key length
    if(strcmp(gp_WIFI_Params->sec, "wep") == 0)
    {
        size = strlen(gp_WIFI_Params->passphrase);
        if((size != 10) && (size != 26) && (size != 5) && (size != 13))
            printf("\n\rInvalid WEP Key length, only 10 or 26 HEX characters allowed (or) 5 or 13 ascii keys allowed");
    }
    
    return result;
}

// Uses Wi-Fi parameters to try and connect
_mqx_uint wifi_param_connect(WIFI_PARAMS_PTR params)
{
    _mqx_uint   result = MQX_OK;
    
    #if USE_ATH_CHANGES
            clear_wep_keys();
    #endif	

    // Check if Wi-Fi already connected
    if(wifi_get_property(gp_WIFI_Params, WIFI_WIFI_CONNECTED))
    {
        // disconnect from current Wi-Fi
        result = disconnect_wifi();
    }
    
    result = iwcfg_set_essid (DEMOCFG_DEFAULT_DEVICE,params->ssid);
    if(result)
        return result;
    
    if ((strcmp(params->sec,"wpa") == 0) || strcmp(params->sec,"wpa2") == 0)
    {
        result = iwcfg_set_passphrase (DEMOCFG_DEFAULT_DEVICE,params->passphrase);
        if(result)
            return result;
        
        result = set_cipher(DEMOCFG_DEFAULT_DEVICE,(char *)CipherName[params->cipher],(char *)CipherName[params->cipher]);
        if(result)
            return result;
    }
    
    result = iwcfg_set_sec_type (DEMOCFG_DEFAULT_DEVICE,params->sec);
    if(result)
        return result;

    if (strcmp(params->sec,"wep") == 0)
    {
        result = set_wep_keys (DEMOCFG_DEFAULT_DEVICE,params->passphrase,
                NULL,NULL,NULL,strlen(params->passphrase),params->wepkey_index);
        if(result)
            return result;

    }
    
    /* Set mode for Wi-Fi */
    if(strcmp(params->mode, "softap") == 0)
        ATH_SET_MODE (DEMOCFG_DEFAULT_DEVICE,"softap");
    else
        result = iwcfg_set_mode (DEMOCFG_DEFAULT_DEVICE, params->mode);
    if(result)
        return result;
        
    printf("\n\rAttempting to connect with these Wi-Fi parameters:\n");
    wifi_param_print(params);
    result = iwcfg_commit (DEMOCFG_DEFAULT_DEVICE);
    if(result)
        return result;
    
    wait_connect(params);
    
    return result;
}

// Prints out Wi-Fi parameters
void wifi_param_print(WIFI_PARAMS_PTR params)
{
    printf("   SSID             = %s\n", params->ssid);
    printf("   Network Mode     = %s\n", params->mode);
    printf("   Security Mode    = %s\n", params->sec);
    if(strcmp(params->sec, "wep") == 0)
    {
      printf("   WEP key          = %s\n", params->passphrase);
      printf("   WEP key index    = %d\n", params->wepkey_index);
    }
    if((strcmp(params->sec, "wpa2") == 0) || (strcmp(params->sec, "wpa") == 0))
    {
      printf("   Passphrase       = %s\n", params->passphrase);
      printf("   cipher           = %s\n", CipherName[params->cipher]);
    }
    printf("\n");
}

_mqx_uint wifi_set_param(WIFI_PARAMS_PTR params, Wifi_Param_Command_t cmd, char * str)
{
    _mqx_uint result = MQX_OK;
    
    switch(cmd)
    {
        case WIFI_SET_SSID:
          if(strlen(str) > WIFI_SSID_LENGTH)
              printf("Invalid SSID length.  Must be %d characters or less.\n", WIFI_SSID_LENGTH);
          else
              strcpy(params->ssid, str);
          break;
          
        case WIFI_SET_MODE:
          if(strcmp(str, "managed") == 0)
              strcpy(params->mode, str);
          else
              printf("Invalid network mode.  Needs to be managed.\n");
          break;
          
        case WIFI_SET_SEC:
          if((strcmp(str, "none") == 0) || (strcmp(str, "wep") == 0) || (strcmp(str, "wpa") == 0) || (strcmp(str, "wpa2") == 0))
              strcpy(params->sec, str);
          else
              printf("Invalid security type.  Needs to be none, wep, wpa, or wpa2.\n");
          break;
          
        case WIFI_SET_PASS:
          if(strlen(str) > WIFI_PASS_LENGTH)
              printf("Invalid passphrase length.  Must be %d characters or less.\n", WIFI_PASS_LENGTH);
          else
              strcpy(params->passphrase, str);
          break;
          
        case WIFI_SET_CIPHER:
          if(strcmp(str, "TKIP") == 0)
              params->cipher = TKIP;
          else if(strcmp(str, "CCMP") == 0)
              params->cipher = CCMP;
          else
              printf("Invalid Cipher setting.  Needs to be TKIP or CCMP\n");
          break;
          
        case WIFI_SET_WEP_KEY:
          if((str[0] >= '1') && (str[0] <= '4'))
              params->wepkey_index = (str[0] - '0');
          else
              printf("Invalid WEP key index.  Needs to be number between 1 and 4\n");
          break;          
    }
    
    return result;
}

_mqx_uint wifi_set_property(WIFI_PARAMS_PTR params, Wifi_Property_t prop, boolean val)
{
    _mqx_uint result = MQX_OK;
    
    switch(prop)
    {
        case WIFI_WIFI_CONNECTED:
          params->wifi_connected = val;
          break;
          
        default:
          result = 1;   // Invalid property
    }
    
    return result;
}

boolean wifi_get_property(WIFI_PARAMS_PTR params, Wifi_Property_t prop)
{
    switch(prop)
    {
        case WIFI_WIFI_CONNECTED:
          return params->wifi_connected;
          
        default:
          return FALSE;
    }
}

_mqx_int disconnect_wifi(void)
{
	_mqx_int result;
        
    result = iwcfg_set_essid (DEMOCFG_DEFAULT_DEVICE,"");
    if(result)
        return result;

    result = iwcfg_commit (DEMOCFG_DEFAULT_DEVICE);
    if(result)
        return result;

    wifi_set_property(gp_WIFI_Params, WIFI_WIFI_CONNECTED, FALSE);
    return MQX_OK;
}

void wifi_flash_read(WIFI_PARAMS_PTR params)
{
    _mqx_int        len = 0;
    
    fseek(flash_file, 1, IO_SEEK_SET);

    len = read(flash_file, params, sizeof(WIFI_PARAMS));
    if (sizeof(WIFI_PARAMS) != len) {
        printf("\nERROR! Could not read Wi-Fi parameters from flash.");
    }
    
}

void wifi_flash_program(WIFI_PARAMS_PTR params)
{
    uint32_t        ioctl_param;
    _mqx_int        len = 0;
    
    /* Move to beginning */
    fseek(flash_file, 1, IO_SEEK_SET);

    /* Unprotecting  the FLASH might be required */
    ioctl_param = 0;
    ioctl(flash_file, FLASH_IOCTL_WRITE_PROTECT, &ioctl_param);

    wifi_flash_erase(params);
    
    len = write(flash_file, params, sizeof(WIFI_PARAMS));
    if (len != sizeof(WIFI_PARAMS)) {
        printf("\nError writing Wi-Fi parameters to flash. Error code: %d", _io_ferror(flash_file));
    }

    wifi_flash_read(params);
    printf("Wi-Fi parameters loaded from flash are:\n");
    wifi_param_print(params);
}

void wifi_flash_erase(WIFI_PARAMS_PTR params)
{
    uint32_t        ioctl_param;
    _mqx_int        error;
    
    /* Move to beginning */
    fseek(flash_file, 1, IO_SEEK_SET);

    /* Unprotecting  the FLASH might be required */
    ioctl_param = 0;
    ioctl(flash_file, FLASH_IOCTL_WRITE_PROTECT, &ioctl_param);

    // erase flash sector
    ioctl(flash_file, FLASH_IOCTL_ERASE_SECTOR, &ioctl_param);
    
    error = _io_ferror(flash_file);
    if(error)
        printf("\nError Erasing Wi-Fi parameters sector in flash. Error code: %d", error);
    else
        printf("Wi-Fi Parameters erased from flash\n");
}

