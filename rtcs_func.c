//
//
//
// FileName: rtcs_func.c
//
//
// Comments:
//
//

#include <string.h>

#include "defines.h"
#include "global.h"
#include "rtcs_func.h"
//#include "httpd_types.h"
#include "tfs.h"
//#include "cgi.h"
//#include "ssi.h"

//#include "EnetHelp_Task.h"
//#include "watchdog_func.h"

//#include  <enetprv.h>
#include  <ipcfg.h>

// Statistics related to messages in support of Dynamic DNS. This global
// variable is maintained by the DynDNS_Task. It is used for debugging.
//
DDNS_STATS  DdnsStats;

uint32_t    hWebServer;

// const HTTPD_ROOT_DIR_STRUCT root_dir[] = 
// {
//     { "", "tfs:" },
//     { 0, 0 }
// };

extern const TFS_DIR_ENTRY   tfs_data[];

void initialize_DHCP_server( void );


void
initialize_networking(void)
{
    int32_t                 error;
    IPCFG_IP_ADDRESS_DATA   ip_data;

//DEBUG - see "rtcs.h" . . . what is the meaning of each config parameter?
//                           how should they be used by this application?
   // runtime RTCS configuration
   _RTCSPCB_init          = 4;
   _RTCSPCB_grow          = 2;
   _RTCSPCB_max           = 8;
   _RTCSTASK_stacksize    = 4096;

    error = RTCS_create();
    if( error != RTCS_OK )
    {
        _task_block();
    }

    _IP_forward = TRUE;

    error = ipcfg_init_device( C450_DEFAULT_ENET_DEVICE, // Ethernet device ID
                               coreDB.mac.addr);         // MAC address

    // Static vs Dynamic IP Address assignment (assigned to the C450)
    //
    // There is a library function; "ipcfg_bind_dhcp_wait()" that can
    //    be used by a DHCP client to get an IP address. The problem with
    //    that function is that while it is waiting, it will not pet
    //    the watchdog. The function "obtain_an_ip_address()" was created
    //    to both pet the watchdog while waiting for a DHCP server to
    //    assign an address, and to periodically give up the CPU while
    //    waiting. This function is used with mode : ENET_ADDR_MODE_AUTO
    //
    switch( coreDB.enet_setup.addr_mode )
    {
        // C450 is the DHCP server (direct connect)
        case ENET_ADDR_MODE_DIRECT: 
            // Set the default IP address, subnet mask, and gateway address.
            ip_data.ip      = DEFAULT_STATIC_IP_ADDR;
            ip_data.mask    = DEFAULT_SUBNET_MASK;
            ip_data.gateway = DEFAULT_GATEWAY;

            ipcfg_bind_staticip( C450_DEFAULT_ENET_DEVICE, &ip_data );
            initialize_DHCP_server();
        break;

        // C450 uses a static IP address
        case ENET_ADDR_MODE_STATIC_IP: 
            // Set user config'd IP address, subnet mask, and gateway address.
            ip_data.ip      = coreDB.enet_setup.static_ip_addr;
            ip_data.mask    = coreDB.enet_setup.subnet_mask;
            ip_data.gateway = coreDB.enet_setup.default_gateway;

            ipcfg_bind_staticip( C450_DEFAULT_ENET_DEVICE, &ip_data );
        break;

        // C450 is a client of a DHCP server
        case ENET_ADDR_MODE_AUTO:
//DEBUG
//            ipcfg_bind_dhcp_wait( C450_DEFAULT_ENET_DEVICE, FALSE, NULL );
            obtain_an_ip_address( C450_DEFAULT_ENET_DEVICE );
    }

    // Ethernet IO Configuration - in addition to the hardware / register
    //   configuration steps that occur as part of the previous call
    //   to "initialize_networking()", Enable the Slew rate and the 
    //   High Drive on all of the ENET RMI Lines. 
    //
    PORTA_PCR15 |= (PORT_PCR_SRE_MASK | PORT_PCR_DSE_MASK);   // TXEN
    PORTA_PCR16 |= (PORT_PCR_SRE_MASK | PORT_PCR_DSE_MASK);   // TXD0
    PORTA_PCR17 |= (PORT_PCR_SRE_MASK | PORT_PCR_DSE_MASK);   // TXD1
    PORTB_PCR0  |= (PORT_PCR_SRE_MASK | PORT_PCR_DSE_MASK);   // MDIO - Phy serial data
    PORTB_PCR1  |= (PORT_PCR_SRE_MASK | PORT_PCR_DSE_MASK);   // MDC - Phy serial clock

    // The "Trivial File System" (tfs) is used to hold the web pages. These
    //   files are encoded as constant arrays, and are located in program
    //   memory. They are hard coded.
    //
    if( (error = _io_tfs_install("tfs:", tfs_data)) ) 
    {
        printf("\ninstall returned: %08x\n", error);
    }

    HTTPSRV_PARAM_STRUCT  srv_param;   // Web server parameters

    _mem_zero( &srv_param, sizeof(HTTPSRV_PARAM_STRUCT) );

    srv_param.port                = coreDB.enet_setup.http_port_number;
    srv_param.ipv4_address.s_addr = INADDR_ANY;
    srv_param.max_uri             = HTTPSRVCFG_DEF_URL_LEN;
    srv_param.max_ses             = HTTPSRVCFG_DEF_SES_CNT;
//    srv_param.cgi_lnk_tbl         = (HTTPSRV_CGI_LINK_STRUCT *) cgi_lnk_tbl;
//    srv_param.ssi_lnk_tbl         = (HTTPSRV_SSI_LINK_STRUCT *) fn_lnk_tbl;
    srv_param.server_prio         = HTTPSRVCFG_DEF_SERVER_PRIO;

    //  Increased "script_stack" to deal with CGI, SSI problems when moving
    //  "large" amounts of data; ie. loading the error web page.
    //
    srv_param.script_stack        = 4096;   //16384;
    srv_param.root_dir            = "tfs:";
    srv_param.index_page          = "\\index.shtm";
    srv_param.auth_table          = NULL;
    srv_param.af                  = AF_INET;

    hWebServer = HTTPSRV_init( &srv_param );
}


//
//  obtain_an_ip_address()
//
void
obtain_an_ip_address( uint32_t device )
{
    uint32_t               result;
    IPCFG_IP_ADDRESS_DATA  auto_ip_data;  // Not actually used

    // See MQXRTCSUG.pdf (RTCS User Guide).
    // There seems to be some support for auto-ip configuration.
    // We are not using that feature, but the call to 
    // "ipcfg_poll_dhcp()" requires that we provide a pointer to
    // a IPCFG_IP_ADDRESS_DATA structure. The user guide describes
    // how this structure might be used. . . for future reference.
    //
    auto_ip_data.ip      = IPADDR(169, 254,   1, 1);
    auto_ip_data.mask    = IPADDR(255, 255,   0, 0) ;
    auto_ip_data.gateway = IPADDR(169, 254,   1, 2);

    do
    {
//        pet_watchdog();

        result = ipcfg_bind_dhcp( device, FALSE );

        if( result == RTCSERR_IPCFG_BUSY )
            _time_delay( 1500 );

    }    while( result != IPCFG_OK );

    do
    {
//        pet_watchdog();

        // The 2nd parameter "FALSE" informs the stack that auto-ip
        //  addressing will not be used.
        //
        result = ipcfg_poll_dhcp( device, FALSE, &auto_ip_data );

        if( result == RTCSERR_IPCFG_BUSY )
            _time_delay( 1500 );

    }    while( result != IPCFG_OK );
}


//
//  initialize_DHCP_server()
//
void 
initialize_DHCP_server( void )
{
    DHCPSRV_DATA_STRUCT  dhcpsrv_data;
    uint8_t              dhcpsrv_options[200];
    _ip_address          routers[1];
    uint8_t *            optptr;
    uint32_t             broadcast_bits, optlen;
    uint32_t             error;

    // Start the DHCP Server, with these parameters;
    //
    //       name:  "DHCP server"
    //   priority: 7
    //      stack: 2000
    //
    error = DHCPSRV_init("DHCP server", 7, 2000);

    if( error != RTCS_OK ) 
    {
        return;
    }

    // Fill in the required parameters:
    // IP Address of the DHCP Server
    dhcpsrv_data.SERVERID = DEFAULT_DHCP_BASE_IP_ADDR; // = 169.254.1.3

    // Infinite lease = 0xFFFFFFFF
    // It is assumed that the units are in seconds. 3600 Seconds = 1 hour
    dhcpsrv_data.LEASE = 3600;

    // Subnet Mask = 255.255.0.0  =  xFF xFF x00 x00 
    dhcpsrv_data.MASK = DEFAULT_SUBNET_MASK;      // = 255.255.0.0

    // TFTP server address:
    dhcpsrv_data.SADDR = DEFAULT_TFTP_IP_ADDR;    // = 169.254.1.4

    memset(dhcpsrv_data.SNAME, 0, sizeof(dhcpsrv_data.SNAME));
    memset(dhcpsrv_data.FILE,  0, sizeof(dhcpsrv_data.FILE));

    // Fill in the options:
    optptr = dhcpsrv_options;
    optlen = sizeof(dhcpsrv_options);

    // Default IP TTL: 
    DHCPSRV_option_int8( &optptr, &optlen, 23, 64 );

    // MTU: 
    DHCPSRV_option_int16( &optptr, &optlen, 26, 1500 );

    // Renewal time: 
    DHCPSRV_option_int32( &optptr, &optlen, 58, 3600 );

    // Rebinding time: 
    DHCPSRV_option_int32( &optptr, &optlen, 59, 5400 );

    // Domain name: 
    DHCPSRV_option_string( &optptr, &optlen, 15, "System450.com" );

    // Broadcast address: 
    // Ex.  DHCP Server IP = 169.254.  1.  3
    //      Subnet Mask    = 255.255.  0.  0
    //      broadcast_bits =   0.  0.255.255
    //
    //      Option 28      = 169.254.255.255
    //
    broadcast_bits = ~DEFAULT_SUBNET_MASK;
    DHCPSRV_option_addr( &optptr, &optlen, 28, DEFAULT_DHCP_BASE_IP_ADDR | broadcast_bits );

    // Router list: It appears that this list should not be empty, otherwise
    //              things don't work right...not sure why.
    routers[0] = DEFAULT_ROUTER;   // = 169.254.1.5

    // The third parameter, value = 3, is the option type identifier
    // The fourth parameter, value = 0, is the list length, add 1 IP addresses
    DHCPSRV_option_addrlist( &optptr, &optlen, 3, routers, 1 );

    // Serve addresses ~.dhcp_server_ip_addr+1 to ~.dhcp_server_ip_addr+4 inclusive,
    //   for a total of 4 client addresses.
    // 
    DHCPSRV_ippool_add( DEFAULT_DHCP_ADDR_POOL, NUM_DHCP_CLIENTS, &dhcpsrv_data, dhcpsrv_options, optptr - dhcpsrv_options );
}


//
//   get_network_speed() - This function uses the Ethernet Handle that was
//                         obtained with the previous call to
//                         "ipcfg_init_device_handle()". This handle contains
//                         function pointers that are used to read the PHY
//                         and determine the network operating speed.
//
int
get_network_speed( void )
{
    _enet_handle     ehandle;
    _rtcs_if_handle  ihandle;
    int              speed = 0;

    ihandle = ipcfg_get_ihandle( C450_DEFAULT_ENET_DEVICE );

    if( ihandle != NULL )
    {
        ehandle = RTCS_get_enet_handle( ihandle );

        if( ehandle != NULL )
            speed = ENET_get_speed( ehandle );
    }

    return( speed );
}

