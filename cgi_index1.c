


#include "hvac.h"
#include "cgi.h"
#include "web_func.h"
#include "global.h"
#include <string.h>
#include <stdlib.h>



const HTTPSRV_SSI_LINK_STRUCT fn_lnk_tbl[] = { { "usb_status_fn", usb_status_fn }, { 0, 0 } };

const HTTPSRV_CGI_LINK_STRUCT cgi_lnk_tbl[] = {
    { "status_data",  cgi_status_data, 0 },
    { 0, 0 }
};


char  cgiResp[1000];

static _mqx_int  
cgi_status_data( HTTPSRV_CGI_REQ_STRUCT * param )
{
    HTTPSRV_CGI_RES_STRUCT response;
    char                   str[60];
    
     if (param->request_method != HTTPSRV_REQ_GET)
        return(0);
     
     int relay_status = get_relay_status();
     
      if(relay_status == 1){
    sprintf( str, "%d\n", 1);
    }else{
       sprintf( str, "%d\n", 0);
    }
      strcat( cgiResp, str ); //add 'str' to 'cgiResp'
      
    response.ses_handle     = param->ses_handle;
    response.content_type   = HTTPSRV_CONTENT_TYPE_PLAIN;
    response.status_code    = 200;
    response.data           = cgiResp;
    response.data_length    = strlen( cgiResp );
    response.content_length = response.data_length;

    HTTPSRV_cgi_write( &response ); 

    return( response.content_length );
}