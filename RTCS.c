/*HEADER**********************************************************************
*
* Copyright 2008 Freescale Semiconductor, Inc.
* Copyright 2004-2008 Embedded Access Inc.
*
* This software is owned or controlled by Freescale Semiconductor.
* Use of this software is governed by the Freescale MQX RTOS License
* distributed with this Material.
* See the MQX_RTOS_LICENSE file distributed for more details.
*
* Brief License Summary:
* This software is provided in source form for you to use free of charge,
* but it is not open source software. You are allowed to use this software
* but you cannot redistribute it or derivative works of it in source form.
* The software may be used only in connection with a product containing
* a Freescale microprocessor, microcontroller, or digital signal processor.
* See license agreement file for full license terms including other
* restrictions.
*****************************************************************************
*
* Comments:
*
*   Example of HVAC using RTCS.
*
*
*END************************************************************************/

#include <mqx.h>
#include <bsp.h>
#include "HVAC.h"
#include "tfs.h"
#include "atheros_main.h"
#include "WiFi_GT202.h"

#include  <ipcfg.h>
#include "httpsrv.h"

HTTPSRV_ALIAS http_aliases[] = {
        { "/usb/", "c:\\" },
        { NULL,    NULL   }
    };

_enet_handle    g_Enet_handle; //EY ADD
extern const SHELL_COMMAND_STRUCT Telnet_commands[];

_mqx_int start_DHCP_server(void);

static void Telnetd_shell_fn (void *dummy) 
{  
	Shell(Telnet_commands,NULL);
}

const RTCS_TASK Telnetd_shell_template = {"Telnet_shell", 8, 2000, Telnetd_shell_fn, NULL};

void HVAC_initialize_networking(void) 
{
    int32_t                  error;
    IPCFG_IP_ADDRESS_DATA   ip_data;
    _enet_address           enet_address;

    atheros_driver_setup();

    /* runtime RTCS configuration for devices with small RAM, for others the default BSP setting is used */
    _RTCSPCB_init = 4;
    _RTCSPCB_grow = 2;
    _RTCSPCB_max = 20;
    _RTCS_msgpool_init = 4;
    _RTCS_msgpool_grow = 2;
    _RTCS_msgpool_max  = 20;
    _RTCS_socket_part_init = 4;
    _RTCS_socket_part_grow = 2;
    _RTCS_socket_part_max  = 20;

    error = RTCS_create();
    
#if RTCSCFG_ENABLE_LWDNS
    LWDNS_server_ipaddr = ENET_IPGATEWAY;
#endif
    
    ip_data.ip      = ENET_IPADDR;
    ip_data.mask    = ENET_IPMASK;
    ip_data.gateway = ENET_IPGATEWAY;
    
    ENET_get_mac_address (DEMOCFG_DEFAULT_DEVICE, ENET_IPADDR, enet_address);
    error = ipcfg_init_device (DEMOCFG_DEFAULT_DEVICE, enet_address);

    if((error = ENET_initialize(DEMOCFG_DEFAULT_DEVICE, enet_address, 0, &g_Enet_handle)) != ENETERR_INITIALIZED_DEVICE)
    {
        while(1)
        {
        	
        }
    }
    
    #if DEMOCFG_ENABLE_DHCP_SERVER
      start_DHCP_server();
    #endif
    
    wifi_params_init();
    set_wifi_callback();
    wifi_param_connect(gp_WIFI_Params);
    
    if( !wifi_get_property(gp_WIFI_Params, WIFI_WIFI_CONNECTED) )
        return; // Wi-Fi not connected, give up on rest of networking initialization

#if DEMOCFG_ENABLE_DHCP    
    /* Bind the IP Address Variables to the Ethernet device */
    
    printf ("Polling DHCP...\n", error);
    do {
        _time_delay (200);
        error = ipcfg_bind_dhcp (DEMOCFG_DEFAULT_DEVICE, TRUE);
    } while (error == IPCFG_ERROR_BUSY);
    
    if (error == IPCFG_ERROR_OK) {
        do {
            _time_delay (200);
            error = ipcfg_poll_dhcp (DEMOCFG_DEFAULT_DEVICE, TRUE, &ip_data);
        }while (error == IPCFG_ERROR_BUSY);
    }
    
    if (error != IPCFG_ERROR_OK) {
        printf ("Error during DHCP bind %08x!\n", error);
    } else {
        printf ("DHCP bind successful.\n");
    }

#else // no DEMOCFG_ENABLE_DHCP    
    error = ipcfg_bind_staticip (DEMOCFG_DEFAULT_DEVICE, &ip_data);
#endif  //DEMOCFG_ENABLE_DHCP    
    
    /* Print IP address */
    ipcfg_get_ip(DEMOCFG_DEFAULT_DEVICE, &ip_data);
    printf ("IP address = %d.%d.%d.%d\n",((ip_data.ip>>24)&0xFF),((ip_data.ip>>16)&0xFF),((ip_data.ip>>8)&0xFF),((ip_data.ip>>0)&0xFF));       

    {
        uint32_t                              server;
        extern const HTTPSRV_CGI_LINK_STRUCT  cgi_lnk_tbl[];
        extern const HTTPSRV_SSI_LINK_STRUCT  fn_lnk_tbl[];
        extern const TFS_DIR_ENTRY            tfs_data[];
        HTTPSRV_PARAM_STRUCT                  params;
        
        error = _io_tfs_install("tfs:", tfs_data);
        if (error) printf("\nTFS install returned: %08x\n", error);

        /* Setup webserver parameters */
        _mem_zero(&params, sizeof(HTTPSRV_PARAM_STRUCT));

    #if RTCSCFG_ENABLE_IP4 
        params.af |= AF_INET;
    #endif

    #if RTCSCFG_ENABLE_IP6 
        params.af |= AF_INET6;
    #endif

        params.root_dir     = "tfs:";
        params.alias_tbl    = (HTTPSRV_ALIAS*)http_aliases;
        params.index_page   = "\\index.shtm";
        params.cgi_lnk_tbl  = (HTTPSRV_CGI_LINK_STRUCT*)cgi_lnk_tbl;
        params.ssi_lnk_tbl  = (HTTPSRV_SSI_LINK_STRUCT*)fn_lnk_tbl;
        params.script_stack = 2500;

        server = HTTPSRV_init(&params);
        if( !server )
        {
            printf("Error: HTTP server init error.\n");
        }
    }
}

_mqx_int start_DHCP_server(void)
{
    DHCPSRV_DATA_STRUCT dhcpsrv_data;
    uchar dhcpsrv_options[200];
    _ip_address routers;
    unsigned char *optptr;
    uint32_t optlen;
    uint32_t error;
    
    /* Start DHCP Server: */
    error = DHCPSRV_init("DHCP server", 7, 2000);
    
    if (error != RTCS_OK) {
        printf("\nFailed to initialize DHCP Server, error %x", error);
        return error;
    }

    printf("\nDHCP Server running");

    /* Fill in the required parameters: */
    dhcpsrv_data.SERVERID = DHCP_SERVER_IP;

    /* Infinite leases: */
    dhcpsrv_data.LEASE = 0xFFFFFFFF;

    /* 255.255.255.0: */
    dhcpsrv_data.MASK = 0xFFFFFF00;

    /* Fill in the options: */
    optptr = dhcpsrv_options;
    optlen = sizeof(dhcpsrv_options);

    /* Default IP TTL: */
    DHCPSRV_option_int8(&optptr, &optlen, 23, 64);

    /* MTU: */
    DHCPSRV_option_int16(&optptr, &optlen, 26, 1500);

    /* Renewal time: */
    DHCPSRV_option_int32(&optptr, &optlen, 58, 3600);

    /* Rebinding time: */
    DHCPSRV_option_int32(&optptr, &optlen, 59, 5400);

    /* Domain name: */
    DHCPSRV_option_string(&optptr, &optlen, 15, "freescaleap.com");

    /* Host name: */
    DHCPSRV_option_string(&optptr, &optlen, 12, "host");

    /* Broadcast address: */
    DHCPSRV_option_addr(&optptr, &optlen, 28, DHCP_BROADCAST_IP);

    /* Router list: */
    routers = DHCP_SERVER_IP;

    DHCPSRV_option_addrlist( &optptr, &optlen, 3, &routers, 1);

    DHCPSRV_ippool_add(DHCP_MIN_IP_ASSIGNED, DHCP_NUM_IPs_ASSIGN, &dhcpsrv_data, dhcpsrv_options, optptr - dhcpsrv_options);
    
    return RTCS_OK;
}

