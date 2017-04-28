
#include "a_types.h"

void atheros_driver_setup(void);
int32_t worker_cmd_handler(int32_t argc, char * argv[]);
void wmiconfig_Task2(uint32_t);
void wmiconfig_Task1(uint32_t);
A_INT32 set_wep_keys 
    (
        A_UINT32 dev_num,
        char* wep_key1,
        char* wep_key2,
        char* wep_key3,
        char* wep_key4,
        A_UINT32 key_len,
        A_UINT32 key_index        
    ); 

