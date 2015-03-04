/**HEADER*******************************************************************
 * * 
 * * Copyright (c) 2008 Freescale Semiconductor;
 * * All Rights Reserved
 * *
 * * Copyright (c) 1989-2008 ARC International;
 * * All Rights Reserved
 * *
 * **************************************************************************** 
 * *
 * * THIS SOFTWARE IS PROVIDED BY FREESCALE "AS IS" AND ANY EXPRESSED OR 
 * * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
 * * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  
 * * IN NO EVENT SHALL FREESCALE OR ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 * * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 * * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
 * * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
 * * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
 * * THE POSSIBILITY OF SUCH DAMAGE.
 * *
 * ****************************************************************************
 * *
 * * Comments:
 * *
 * *   This file contains main initialization for your application
 * *   and infinite loop
 * *
 * *END************************************************************************/

#include "main.h"
#include <tfs.h>
#include <httpd.h>
#include <httpd_types.h>


#if defined(APPLICATION_HAS_SHELL) && (!SHELLCFG_USES_RTCS)
#error This application requires SHELLCFG_USES_RTCS defined non-zero in user_config.h. Please recompile libraries with this option if any Ethernet interface is available.
#endif

TASK_TEMPLATE_STRUCT MQX_template_list[] =
{
/*  Task number, Entry point, Stack, Pri, String, Auto? */
   {MAIN_TASK,   Main_task,   2000,  9,   "main", MQX_AUTO_START_TASK},
   {0,           0,           0,     0,   0,      0,                 }
};

/*TASK*-----------------------------------------------------------------
 * *
 * * Function Name  : Main_task
 * * Comments       :
 * *    This task initializes MFS and starts SHELL.
 * *
 * *END------------------------------------------------------------------*/

//HMI_CLIENT_STRUCT_PTR hmi_client; // declares hmi_client, needs to be up here for some reason -- don't remove this
unsigned char http_refresh_text[] = "<!DOCTYPE html>\
<html>\
<head>\
<title>Muahahahaaa!</title>\
</head>\
<body>\
	<h1>Stefan Taylor</h1>\
    <h2>Embedded Systems</h2>\
	<p>This a website made by the Tower!!!!</p>\
	<ul>\
		<li><a href=\"\">List Element</a></li>\
		<li><a href=\"192.168.105.203\">List Element</a></li>\
		<li><a href=\"192.168.105.203\">List Element</a></li>\
		<li><a href=\"192.168.105.203\">List Element</a></li>\
	</ul>\
</body>\
</html>";
static HTTPD_ROOT_DIR_STRUCT http_root_dir[] = {{"", "tfs:"}, { 0,0 }};
const TFS_DIR_ENTRY static_data[] = {{"/index.html", 0, http_refresh_text,sizeof(http_refresh_text)},{0,0,0,0}};
HTTPD_STRUCT *http_server;

void Main_task(uint_32 initial_data){
//	hmi_client = _bsp_btnled_init(); // get's value of hmi_client from init function -- don't remove this

	rtcs_init();

	_io_tfs_install("tfs:", static_data);
	
	http_server = httpd_server_init_af(http_root_dir, "\\index.html", AF_INET);

	httpd_server_run(http_server);
	
	while(1){
		ipcfg_task_poll();
	}



}
/* EOF */
