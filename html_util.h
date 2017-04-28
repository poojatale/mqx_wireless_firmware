#ifndef _html_util_h_
#define _html_util_h_
/**HEADER********************************************************************
* 
* Copyright (c) 2004-2008 Embedded Access Inc.;
* All Rights Reserved
*
* $FileName: html_util.h
*
*END************************************************************************/
#include <httpsrv.h>
#include "web_func.h"

uint32_t  load_web_page( HTTPSRV_CGI_RES_STRUCT * resp, char * url );

uint32_t  success_login( HTTPSRV_CGI_RES_STRUCT * resp, UID_LOGIN * uid );

uint32_t  load_error_page( HTTPSRV_CGI_RES_STRUCT * resp, 
                           char * prompt, 
                           char * error_msg, 
                           char * next_url );

#endif
