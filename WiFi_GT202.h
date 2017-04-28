
#include "a_types.h"

#define WIFI_SSID_LENGTH        28
#define WIFI_MODE_LENGTH        8
#define WIFI_SEC_LENGTH         8
#define WIFI_PASS_LENGTH        28
#define WIFI_CIPHER_LENGTH      4

typedef enum {
   TKIP = 0,
   CCMP
} Cipher_t;

extern const char * CipherName[];

typedef enum {
   WIFI_SET_SSID = 0,
   WIFI_SET_MODE,
   WIFI_SET_SEC,
   WIFI_SET_PASS,
   WIFI_SET_CIPHER,
   WIFI_SET_WEP_KEY
} Wifi_Param_Command_t;

typedef enum {
   WIFI_WIFI_CONNECTED = 0,
} Wifi_Property_t;

typedef struct {
    char ssid[WIFI_SSID_LENGTH];
    char mode[WIFI_MODE_LENGTH];
    char sec[WIFI_SEC_LENGTH];
    char passphrase[WIFI_PASS_LENGTH];
    Cipher_t cipher;
    char wepkey_index;
    boolean wifi_connected;
} WIFI_PARAMS, * WIFI_PARAMS_PTR;  

_mqx_uint wifi_param_connect(WIFI_PARAMS_PTR params);
void wifi_param_print(WIFI_PARAMS_PTR params);
_mqx_uint wifi_set_param(WIFI_PARAMS_PTR params, Wifi_Param_Command_t cmd, char * str);
_mqx_uint wifi_set_property(WIFI_PARAMS_PTR params, Wifi_Property_t prop, boolean val);
boolean wifi_get_property(WIFI_PARAMS_PTR params, Wifi_Property_t prop);
void wifi_flash_erase(WIFI_PARAMS_PTR params);
void wifi_flash_program(WIFI_PARAMS_PTR params);
_mqx_uint wifi_params_init(void);
A_INT32 set_wifi_callback();

extern   WIFI_PARAMS_PTR gp_WIFI_Params;
