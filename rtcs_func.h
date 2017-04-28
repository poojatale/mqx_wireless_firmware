#include <mqx.h>
#include <bsp.h>
#include <rtcs.h>
#include <httpsrv.h>

#ifndef  __rtcs_func_inc
#define  __rtcs_func_inc

//
//  SUCCESS_FAIL_COUNT - This data structure contains "success" and 
//                       "failure" counters. It is used by the DDNS_STATS
//                       type to track the success / failure of 
//                       various messages.
//
typedef struct 
{
    uint32_t  success;       // Success counter
    uint32_t  fail;          // Failure counter

}   SUCCESS_FAIL_COUNT;

//
//  DDNS_STATS - This data structure contains information about the success
//               and failure count of the various messages that are used
//               in support of Dynamic DNS. These messages include requests
//               of a DNS server to provide an IP address for a host name,
//               checking the external IP address of a router, and updating
//               the IP address of a dynamic host name at the DynDNS.com
//               web server. Each time one of these messages is attempted
//               either a success or fail counter for the message will be
//               incremented.
//
typedef struct
{
    // Connect attempts; checkip.dyndns.com
    SUCCESS_FAIL_COUNT  connect_checkip;

    // Check IP request/response results, checkip_dyndns.com
    SUCCESS_FAIL_COUNT  checkip; 

    // Connect attempts; members.dyndns.org
    SUCCESS_FAIL_COUNT  connect_updateip;

    // Update IP request/response results, members.dyndns.org
    SUCCESS_FAIL_COUNT  updateip;

    SUCCESS_FAIL_COUNT  dns_1;     // Primary DNS server, user supplied

    SUCCESS_FAIL_COUNT  dns_2;     // Secondary DNS server, 
                                   //    Google, IP = 8.8.8.8

    SUCCESS_FAIL_COUNT  dns_3;     // Third DNS server, 
                                   //    Google, IP = 8.8.4.4

}    DDNS_STATS;

extern DDNS_STATS DdnsStats;

// Google's Public DNS Servers = 8.8.8.8  and  8.8.4.4
//
#define GOOGLE_DNS_SERVER_1 IPADDR( 8, 8, 8, 8 )
#define GOOGLE_DNS_SERVER_2 IPADDR( 8, 8, 4, 4 )

void     initialize_networking( void );
void     obtain_an_ip_address( uint32_t device );

int      get_network_speed( void );


#ifndef ENET_IPADDR
    #define ENET_IPADDR  IPADDR(192,168,1,4)
 #endif

 #ifndef ENET_IPMASK
    #define ENET_IPMASK  IPADDR(255,255,255,0)
 #endif

 #ifndef ENET_IPGATEWAY
    #define ENET_IPGATEWAY  IPADDR(192,168,1,1)
 #endif

#ifndef C450_DEFAULT_ENET_DEVICE
   #define C450_DEFAULT_ENET_DEVICE   BSP_DEFAULT_ENET_DEVICE
#endif

#define DHCP_SERVER_IP  IPADDR(192,168,1,5)
#define ROUTER_1_IP     IPADDR(192,168,1,6)
#define ROUTER_2_IP     IPADDR(192,168,1,7)

#endif