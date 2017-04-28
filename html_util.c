//
// (C)Copyright Johnson Controls, Inc. Use or copying of all or any part of 
// the document, except as permitted by the License Agreement, is prohibited.
//
//  FILENAME :  html_util.c
//
//  PURPOSE :   This file contains functions to build several different
//              key pieces of a web page. These functions are used when
//              those pages, or parts of pages, are generated dynamically
//              at run time.
//
// History :
// Date        Author     Rel      EC#    Prob#  Task# Reason for change
// ---------   --------- ------- ------- ------- ----- --------------------
//

#include <string.h>

#include "defines.h"
#include "html_util.h"
#include "web_func.h"


// Three constant strings that provide the bulk of the HTML that form
// the "Load Web Page" response. This response forwards the client
// browser to a new page.
//
const char strLoadWebPage1[] = 
 "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\"> \n\
  <html xmlns=\"http://www.w3.org/1999/xhtml\"> \n\
  <head> \n\
  <meta content=\"text/html; charset=utf-8\" http-equiv=\"Content-Type\" />\n";

const char strLoadWebPage2[] = "<meta http-equiv=\"REFRESH\" content=\"0;url=";

const char strLoadWebPage3[] = "\"><title></title></head><body></body></html>";

// Three constant strings that provide the bulk of the HTML that form
// the "Successful Login" response.
//
const char strSuccessLogin1[] = 
 "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\"> \n\
  <html xmlns=\"http://www.w3.org/1999/xhtml\"> \n\
  <head> \n\
  <meta content=\"text/html; charset=utf-8\" http-equiv=\"Content-Type\" />\n";

const char strSuccessLogin2[] = "<meta http-equiv=\"REFRESH\" content=\"0;";

const char strSuccessLogin3[] = 
 "\"/> \n\
  <title>Login Ok</title> \n\
  </head><body></body></html>\n";


// Three constant strings that provide the bulk of the HTML that form
// the Error web page.
//
const char strError1[] = 
 "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\"> \n\
  <html xmlns=\"http://www.w3.org/1999/xhtml\"> \n\
  <head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"/><title>Configuration Error</title> \n\
  <style type=\"text/css\" title=\"Master\"> \n\
  html {background:#0071BC url(gradient.png) top left repeat-x;} \n\
  body {font-family:Arial, Verdana, Helvetica, sans-serif; font-size:100%; color:#000; margin:0 auto; padding:0; width:100%;} \n\
  h1 {font-size:1em; font-weight:bold; padding:0; margin:0;} \n\
  h2 {font-size:1em; font-weight:bold; padding:0; margin:0;} \n\
  h3 {background-color: #FFFF00; text-align: left; font-size:.95em; font-weight:bold; padding:0 0 0 10px; margin:0; line-height:1.2em;} \n\
  input {font-size:1em; width:140px; margin:0 0 5px 10px;} \n\
  .clear {clear:both; height:1px;} \n\
  * html .clear {clear:both; height:1px; margin:0; padding:0;} \n\
  .topPadding {padding:8px 0 0 0;} \n\
  .topPadding2 {padding:16px 0 0 0;} \n\
  #mainContainer {width:772px; margin:20px auto;} \n\
  #header {background:#fff; width:590px; padding:10px; float:right;} \n\
  * html #header {padding:10px 10px 0 10px;} \n\
  #header ul {list-style:none; margin:0; padding:0;} \n\
  #header li {float:left;} \n\
  #header li img{padding-right:200px; vertical-align:middle;} \n\
  .systemTitle {font-size:3em; font-weight:bold; padding:0 15px 0 0;} \n\
  .systemSubTitle {font-size:1em;} \n\
  #mainBody {width:770px; margin-top:10px;} \n\
  #sidebar {text-align: center; background:#fff; width:130px; padding:10px; float:left;} \n\
  #sidebar ul {list-style:none; margin:2px 0 20px 0; padding:0;} \n\
  #sidebar li {list-style:none; font-size:.76em; line-height:1.8em;} \n\
  #blankbar {width:590px; padding:2px; float:right;} \n\
  #mainContent {background:#fff; width:590px; padding:10px; float:right;} \n\
  #mainContent ul {list-style:none; margin:10px 0 0 0; padding:0;} \n\
  #mainContent li {float:left; margin-right:20px; margin-bottom:5px;} \n\
  .contentBorder {border-color:#999; border-style:solid; border-width:thin; margin:10px 0;} \n\
  .menubutton {font-size:1.1em; width:80px; margin:5px 5px 5px 5px;} \n\
  .heightSpacer {height:51px;} \n\
  .leftcolumn {float:left; margin:0 0 0 15px; width:90%;} \n\
  .rightcolumn {float:right; margin:0 0 0 15px; width:25%;} \n\
  .midcolumn {float:left; font-size:1.2em; vertical-align:text-bottom; margin:10px 10px 10px 10px; width:90%;} \n\
  </style> \n\
  <script type=\"text/javascript\"> \n\
  function next_url(){ location.href = \"";

//
// The next URL goes between strError1 and strError2. This value is not fixed.
//
const char strError2[] = 
 "\";  } \n\
  </script> \n\
  </head> \n\
  <body style=\"height: 749px\"><div id=\"mainContainer\"><div id=\"mainBody\" style=\"height: 646px\"> \n\
  <div id=\"sidebar\" style=\"height: 181px\"> \n\
  <img src=\"jci_pennLogo.gif\" title=\"Johnson Controls/PENN\" alt=\"Johnson Controls/PENN\" style=\"border-width: 0; vertical-align: middle;\"/> \n\
  <br/><br/><br/><br/></div> \n\
  <div id=\"header\" style=\"height: 61px\"><ul><li></li> \n\
  <li class=\"topPadding\" style=\"width: 321px\"><span class=\"systemTitle\">&nbsp; System 450</span></li> \n\
  <li class=\"topPadding2\"><span class=\"systemSubTitle\"></span></li> \n\
  </ul><div class=\"clear\"></div></div> \n\
  <div id=\"blankbar\" style=\"height: 8px\"></div> \n\
  <div id=\"mainContent\" style=\"height: 475px\"> \n\
  <h1>";
//
// The error specific Prompt goes between strError2 and strError3.
//
const char strError3[] = 
 "</h1> \n\
  <div class=\"contentBorder\" style=\"height: 390px\" > \n\
  <div class=\"midcolumn\"><img src=\"warning30.png\" alt=\"X\"/>&nbsp;&nbsp;&nbsp;User entry error</div> \n\
  <div class=\"leftcolumn\">\n";
//
// The error specific Message goes between strError3 and strError4.
//
const char strError4[] = 
 "</div></div><div class=\"rightcolumn\"> \n\
  <input class=\"menubutton\" name=\"ButtonOk\" type=\"button\" value=\"Ok\" onclick=\"next_url()\"/> \n\
  </div></div><div class=\"clear\">&nbsp;</div></div></div></body></html>\n";


//
//   load_web_page() - This function is passed a pointer to a CGI response
//                     structure. It uses this parameter to send a response
//                     to the client browser.
//
//   Note: It is assumed that the following fields have been properly
//         established BEFORE this function is called;
//
//           response.ses_handle   = session handle
//           response.status_code  = ie, 200;
//
uint32_t
load_web_page( HTTPSRV_CGI_RES_STRUCT * resp, char * url )
{
    resp->content_type    = HTTPSRV_CONTENT_TYPE_HTML;

    resp->content_length  = strlen( strLoadWebPage1 );
    resp->content_length += strlen( strLoadWebPage2 );
    resp->content_length += strlen( url );
    resp->content_length += strlen( strLoadWebPage3 );

    resp->data           = (char *) strLoadWebPage1;
    resp->data_length    = strlen( strLoadWebPage1 );
    HTTPSRV_cgi_write( resp ); 

    resp->data           = (char *) strLoadWebPage2;
    resp->data_length    = strlen( strLoadWebPage2 );
    HTTPSRV_cgi_write( resp ); 

    resp->data           = url;
    resp->data_length    = strlen( url );
    HTTPSRV_cgi_write( resp ); 

    resp->data           = (char *) strLoadWebPage3;
    resp->data_length    = strlen( strLoadWebPage3 );
    HTTPSRV_cgi_write( resp ); 

    return( resp->content_length );        
}

//
//   success_login() - This function builds and loads a simple web page,
//                     and then redirects the client browser to 
//                     "overview.htm", attaching the uid string to the URL.
//                     of that page.
//
uint32_t
success_login( HTTPSRV_CGI_RES_STRUCT * resp, UID_LOGIN * uid )
{
    char    url[UID_RANDOM_LENGTH+50];

    sprintf( url, "url=overview.shtm?%s", uid->str );

    resp->content_type    = HTTPSRV_CONTENT_TYPE_HTML;

    resp->content_length  = strlen( strSuccessLogin1 );
    resp->content_length += strlen( strSuccessLogin2 );
    resp->content_length += strlen( url );
    resp->content_length += strlen( strSuccessLogin3 );

    resp->data           = (char *) strSuccessLogin1;
    resp->data_length    = strlen( strSuccessLogin1 );
    HTTPSRV_cgi_write( resp ); 

    resp->data           = (char *) strSuccessLogin2;
    resp->data_length    = strlen( strSuccessLogin2 );
    HTTPSRV_cgi_write( resp ); 

    resp->data           = url;
    resp->data_length    = strlen( url );
    HTTPSRV_cgi_write( resp ); 

    resp->data           = (char *) strSuccessLogin3;
    resp->data_length    = strlen( strSuccessLogin3 );
    HTTPSRV_cgi_write( resp ); 

    return( resp->content_length );        
}

//
//  load_error_page() - This function causes an entire web page to be
//                      displayed. This particular page is used when there
//                       is some sort of invalid input in an output setup,
//                       sensor setup, TCP/IP setup, or login attempt.
//
//      NOTE: You can view the file "error_template.htm" with an
//            HTML editor to see what this page looks like. If you 
//            change "error_template.htm" and want those changes to
//            be in the firmware, you will need to update the source
//            code below such that it matches "error_tempalte.htm"
//
//   Parameters :  session  -  Pointer to the HTTP session
// 
//
uint32_t 
load_error_page( HTTPSRV_CGI_RES_STRUCT * resp, 
                 char * prompt, 
                 char * error_msg, 
                 char * next_url )
{
    resp->content_type    = HTTPSRV_CONTENT_TYPE_HTML;

    resp->content_length  = strlen( strError1 );
    resp->content_length += strlen( strError2 );
    resp->content_length += strlen( strError3 );
    resp->content_length += strlen( strError4 );
    resp->content_length += strlen( prompt    );
    resp->content_length += strlen( error_msg );
    resp->content_length += strlen( next_url  );

    resp->data           = (char *) strError1;
    resp->data_length    = strlen( strError1 );
    HTTPSRV_cgi_write( resp ); 

    resp->data           = next_url;
    resp->data_length    = strlen( next_url );
    HTTPSRV_cgi_write( resp ); 

    resp->data           = (char *) strError2;
    resp->data_length    = strlen( strError2 );
    HTTPSRV_cgi_write( resp ); 

    resp->data           = prompt;
    resp->data_length    = strlen( prompt );
    HTTPSRV_cgi_write( resp ); 

    resp->data           = (char *) strError3;
    resp->data_length    = strlen( strError3 );
    HTTPSRV_cgi_write( resp ); 

    resp->data           = error_msg;
    resp->data_length    = strlen( error_msg );
    HTTPSRV_cgi_write( resp ); 

    resp->data           = (char *) strError4;
    resp->data_length    = strlen( strError4 );
    HTTPSRV_cgi_write( resp ); 

    return( resp->content_length );        
}
