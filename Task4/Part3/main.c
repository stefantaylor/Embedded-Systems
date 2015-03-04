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

//Variable Declarations

HMI_CLIENT_STRUCT_PTR hmi_client;
unsigned char http_refresh_text[] = "<!DOCTYPE html>\
<html>\
<head>\
<title>Stefan's Super Slick, Slightly Silly, Still Stupendous, Sensor System</title>\
</head>\
<body>\
	<h1>Stefan Taylor</h1>\
    <h2>Embedded Systems</h2>\
	<p>Alarm System</p>\
	<ul>\
		<li><a href=\"/alarm.cgi?1\">Arm/Disarm System</a></li>\
\
		<li><a href=\"/alarm.cgi?2\">Reset Alarms</a></li>\
\
		<li><a href=\"/alarm.cgi?3\">Arm/Disarm Sensor 1</a></li>\
		<li><a href=\"/alarm.cgi?4\">Arm/Disarm Sensor 2</a></li>\
		<li><a href=\"/alarm.cgi?5\">Arm/Disarm Sensor 3</a></li>\
		<li><a href=\"/alarm.cgi?6\">Arm/Disarm Sensor 4</a></li>\
\
		<li><a href=\"/scu.cgi?00:00:00\">Set Current Time</a></li>\
		<li><a href=\"/rcu.cgi\">Retrieve Current Time</a></li>\
\
		<li><a href=\"/s1en.cgi?00:00:00\">Set Sensor 1 Enable Time</a></li>\
		<li><a href=\"/s1di.cgi?99:99:99\">Set Sensor 1 Disable Time</a></li>\
\
		<li><a href=\"/s2en.cgi?00:00:00\">Set Sensor 2 Enable Time</a></li>\
		<li><a href=\"/s2di.cgi?99:99:99\">Set Sensor 2 Disable Time</a></li>\
\
		<li><a href=\"/s3en.cgi?00:00:00\">Set Sensor 3 Enable Time</a></li>\
		<li><a href=\"/s3di.cgi?99:99:99\">Set Sensor 3 Disable Time</a></li>\
\
		<li><a href=\"/s4en.cgi?00:00:00\">Set Sensor 4 Enable Time</a></li>\
		<li><a href=\"/s4di.cgi?99:99:99\">Set Sensor 4 Disable Time</a></li>\
	</ul>\
</body>\
</html>";
static HTTPD_ROOT_DIR_STRUCT http_root_dir[] = {{"", "tfs:"}, { 0,0 }};
const TFS_DIR_ENTRY static_data[] = {{"/index.html", 0, http_refresh_text,sizeof(http_refresh_text)},{0,0,0,0}};
HTTPD_STRUCT *http_server;
RTC_TIME_STRUCT curr_time, time;
char buffer[2000];
int hours, minutes, seconds;

RTC_TIME_STRUCT s1en, s2en, s3en, s4en, s1di, s2di, s3di, s4di;
int s1enset = 0, s2enset = 0, s3enset = 0, s4enset = 0, s1diset = 0, s2diset = 0, s3diset = 0, s4diset = 0;

int armed1;
int armed2;
int armed3;
int armed4;
int tripped1;
int tripped2;
int tripped3;
int tripped4;
int count;

//Function Definitions

_mqx_int alarm_callback(HTTPD_SESSION_STRUCT *session){
	int op =  atoi(session->request.urldata);
	if (op == 1){
		// Arm/Disarm System
		if (armed1 + armed2 + armed3 + armed4 == 4){
			armed1 = 0;
			armed2 = 0;
			armed3 = 0;
			armed4 = 0;
			btnled_set_value(hmi_client, HMI_LED_1, HMI_VALUE_OFF);
			btnled_set_value(hmi_client, HMI_LED_2, HMI_VALUE_OFF);
			btnled_set_value(hmi_client, HMI_LED_3, HMI_VALUE_OFF);
			btnled_set_value(hmi_client, HMI_LED_4, HMI_VALUE_OFF);
			httpd_sendstr(session->sock, "<html><body>System Disarmed<br><a href=http://192.168.105.203/>Return</a></body></html>");
		}
		else {
			armed1 = 1;
			armed2 = 1;
			armed3 = 1;
			armed4 = 1;
			btnled_set_value(hmi_client, HMI_LED_1, HMI_VALUE_ON);
			btnled_set_value(hmi_client, HMI_LED_2, HMI_VALUE_ON);
			btnled_set_value(hmi_client, HMI_LED_3, HMI_VALUE_ON);
			btnled_set_value(hmi_client, HMI_LED_4, HMI_VALUE_ON);
			httpd_sendstr(session->sock, "<html><body>System Armed<br><a href=http://192.168.105.203/>Return</a></body></html>");
		}
		return session->request.content_len;
	}
	if (op == 2){
		// Hush System
		if (armed1){
			tripped1 = 0;
			btnled_set_value(hmi_client, HMI_LED_1, HMI_VALUE_ON);
		}
		if (armed2){
			tripped2 = 0;
			btnled_set_value(hmi_client, HMI_LED_2, HMI_VALUE_ON);
		}
		if (armed3){
			tripped3 = 0;
			btnled_set_value(hmi_client, HMI_LED_3, HMI_VALUE_ON);
		}
		if (armed4){
			tripped4 = 0;
			btnled_set_value(hmi_client, HMI_LED_4, HMI_VALUE_ON);
		}
		httpd_sendstr(session->sock, "<html><body>Alarms Reset<br><a href=http://192.168.105.203/>Return</a></body></html>");
		return session->request.content_len;
	}
	if (op == 3){
		// Arm/Disarm Sensor 1
		if (armed1){
			armed1 = 0;
			btnled_set_value(hmi_client, HMI_LED_1, HMI_VALUE_OFF);	
			httpd_sendstr(session->sock, "<html><body>Sensor 1 Disarmed<br><a href=http://192.168.105.203/>Return</a></body></html>");		
		}
		else {
			armed1 = 1;
			btnled_set_value(hmi_client, HMI_LED_1, HMI_VALUE_ON);	
			httpd_sendstr(session->sock, "<html><body>Sensor 1 Armed<br><a href=http://192.168.105.203/>Return</a></body></html>");		
		}
		return session->request.content_len;
	}
	if (op == 4){
		// Arm/Disarm Sensor 2
		if (armed2){
			armed2 = 0;
			btnled_set_value(hmi_client, HMI_LED_2, HMI_VALUE_OFF);	
			httpd_sendstr(session->sock, "<html><body>Sensor 2 Disarmed<br><a href=http://192.168.105.203/>Return</a></body></html>");		
		}
		else {
			armed2 = 1;
			btnled_set_value(hmi_client, HMI_LED_2, HMI_VALUE_ON);	
			httpd_sendstr(session->sock, "<html><body>Sensor 2 Armed<br><a href=http://192.168.105.203/>Return</a></body></html>");			
		}
		return session->request.content_len;
	}
	if (op == 5){
		// Arm/Disarm Sensor 3
		if (armed3){
			armed3 = 0;
			btnled_set_value(hmi_client, HMI_LED_3, HMI_VALUE_OFF);	
			httpd_sendstr(session->sock, "<html><body>Sensor 3 Disarmed<br><a href=http://192.168.105.203/>Return</a></body></html>");		
		}
		else {
			armed3 = 1;
			btnled_set_value(hmi_client, HMI_LED_3, HMI_VALUE_ON);	
			httpd_sendstr(session->sock, "<html><body>Sensor 3 Armed<br><a href=http://192.168.105.203/>Return</a></body></html>");			
		}
		return session->request.content_len;
	}
	if (op == 6){
		// Arm/Disarm Sensor 4
		if (armed4){
			armed4 = 0;
			btnled_set_value(hmi_client, HMI_LED_4, HMI_VALUE_OFF);
			httpd_sendstr(session->sock, "<html><body>Sensor 4 Disarmed<br><a href=http://192.168.105.203/>Return</a></body></html>");			
		}
		else {
			armed4 = 1;
			btnled_set_value(hmi_client, HMI_LED_4, HMI_VALUE_ON);	
			httpd_sendstr(session->sock, "<html><body>Sensor 4 Armed<br><a href=http://192.168.105.203/>Return</a></body></html>");			
		}
		return session->request.content_len;
	}
	

}

_mqx_int scu_callback(HTTPD_SESSION_STRUCT *session){
	sscanf(session->request.urldata, "%u:%u:%u", &hours, &minutes, &seconds);
	time.seconds = seconds+minutes*60+hours*3600;
	_rtc_set_time(&time);
	httpd_sendstr(session->sock, "<html><body>Your new time has been set.<br><a href=http://192.168.105.203/>Return</a></body></html>");	
	return session->request.content_len;
}


_mqx_int rcu_callback(HTTPD_SESSION_STRUCT *session){
	_rtc_get_time(&curr_time);
	seconds = curr_time.seconds;
	minutes = seconds/60;
	hours = minutes/60;
	seconds = seconds - minutes *60;
	minutes = minutes - hours * 60;

	sprintf(buffer, "<html><body>%u:%u:%u<br><a href=http://192.168.105.203/>Return</a></body></html>", hours, minutes, seconds);
	httpd_sendstr(session->sock, buffer);
	return session->request.content_len;
}


_mqx_int s1en_callback(HTTPD_SESSION_STRUCT *session){
	sscanf(session->request.urldata, "%u:%u:%u", &hours, &minutes, &seconds);
	s1en.seconds = seconds+minutes*60+hours*3600;
	s1enset = 1;
	httpd_sendstr(session->sock, "<html><body>An enable time has been set for sensor 1.<br><a href=http://192.168.105.203/>Return</a></body></html>");	
	return session->request.content_len;
}


_mqx_int s1di_callback(HTTPD_SESSION_STRUCT *session){
	sscanf(session->request.urldata, "%u:%u:%u", &hours, &minutes, &seconds);
	s1di.seconds = seconds+minutes*60+hours*3600;
	s1diset = 1;
	httpd_sendstr(session->sock, "<html><body>A disable time has been set for sensor 1.<br><a href=http://192.168.105.203/>Return</a></body></html>");	
	return session->request.content_len;
}

_mqx_int s2en_callback(HTTPD_SESSION_STRUCT *session){
	sscanf(session->request.urldata, "%u:%u:%u", &hours, &minutes, &seconds);
	s2en.seconds = seconds+minutes*60+hours*3600;
	s2enset = 1;
	httpd_sendstr(session->sock, "<html><body>An enable time has been set for sensor 2.<br><a href=http://192.168.105.203/>Return</a></body></html>");	
	return session->request.content_len;
}


_mqx_int s2di_callback(HTTPD_SESSION_STRUCT *session){
	sscanf(session->request.urldata, "%u:%u:%u", &hours, &minutes, &seconds);
	s2di.seconds = seconds+minutes*60+hours*3600;
	s2diset = 1;
	httpd_sendstr(session->sock, "<html><body>A disable time has been set for sensor 2.<br><a href=http://192.168.105.203/>Return</a></body></html>");	
	return session->request.content_len;
}

_mqx_int s3en_callback(HTTPD_SESSION_STRUCT *session){
	sscanf(session->request.urldata, "%u:%u:%u", &hours, &minutes, &seconds);
	s3en.seconds = seconds+minutes*60+hours*3600;
	s3enset = 1;
	httpd_sendstr(session->sock, "<html><body>An enable time has been set for sensor 3.<br><a href=http://192.168.105.203/>Return</a></body></html>");	
	return session->request.content_len;
}


_mqx_int s3di_callback(HTTPD_SESSION_STRUCT *session){
	sscanf(session->request.urldata, "%u:%u:%u", &hours, &minutes, &seconds);
	s3di.seconds = seconds+minutes*60+hours*3600;
	s3diset = 1;
	httpd_sendstr(session->sock, "<html><body>A disable time has been set for sensor 3.<br><a href=http://192.168.105.203/>Return</a></body></html>");	
	return session->request.content_len;
}

_mqx_int s4en_callback(HTTPD_SESSION_STRUCT *session){
	sscanf(session->request.urldata, "%u:%u:%u", &hours, &minutes, &seconds);
	s4en.seconds = seconds+minutes*60+hours*3600;
	s4enset = 1;
	httpd_sendstr(session->sock, "<html><body>An enable time has been set for sensor 4.<br><a href=http://192.168.105.203/>Return</a></body></html>");	
	return session->request.content_len;
}


_mqx_int s4di_callback(HTTPD_SESSION_STRUCT *session){
	sscanf(session->request.urldata, "%u:%u:%u", &hours, &minutes, &seconds);
	s4di.seconds = seconds+minutes*60+hours*3600;
	s4diset = 1;
	httpd_sendstr(session->sock, "<html><body>A disable time has been set for sensor 4.<br><a href=http://192.168.105.203/>Return</a></body></html>");	
	return session->request.content_len;
}


HTTPD_CGI_LINK_STRUCT http_cgi_params[] = { { "alarm", alarm_callback }, 
{ "scu", scu_callback }, 
{ "rcu", rcu_callback }, 
{ "s1en", s1en_callback }, 
{ "s1di", s1di_callback },
{ "s2en", s2en_callback }, 
{ "s2di", s2di_callback },
{ "s3en", s3en_callback }, 
{ "s3di", s3di_callback },
{ "s4en", s4en_callback }, 
{ "s4di", s4di_callback },
{0,0}};

void button1_push(void *ptr){
	//trips alarm 1
	if (armed1){
		tripped1 = 1;
		btnled_set_value(hmi_client, HMI_LED_1, HMI_VALUE_OFF);
		s1enset = 0;
		s1diset = 0;
	}
}


void button2_push(void *ptr){
	//trips alarm 2
	if (armed2){	
		tripped2 = 1;
		btnled_set_value(hmi_client, HMI_LED_2, HMI_VALUE_OFF);
		s2enset = 0;
		s2diset = 0;
	}
}

void button3_push(void *ptr){
	//trips alarm 3
	if (armed3){
		tripped3 = 1;
		btnled_set_value(hmi_client, HMI_LED_3, HMI_VALUE_OFF);
		s3enset = 0;
		s3diset = 0;
	}
}

void button4_push(void *ptr){
	//trips alarm 4
	if (armed4){
		tripped4 = 1;
		btnled_set_value(hmi_client, HMI_LED_4, HMI_VALUE_OFF);
		s4enset = 0;
		s4diset = 0;
	}
}

void button5_push(void *ptr){
	//resets any tripped alarms
	if (armed1){
		tripped1 = 0;
		btnled_set_value(hmi_client, HMI_LED_1, HMI_VALUE_ON);
	}
	if (armed2){
		tripped2 = 0;
		btnled_set_value(hmi_client, HMI_LED_2, HMI_VALUE_ON);
	}
	if (armed3){
		tripped3 = 0;
		btnled_set_value(hmi_client, HMI_LED_3, HMI_VALUE_ON);
	}
	if (armed4){
		tripped4 = 0;
		btnled_set_value(hmi_client, HMI_LED_4, HMI_VALUE_ON);
	}
}

void button6_push(void *ptr){
	//arms/disarms the alarm system
	if (armed1 + armed2 + armed3 + armed4 == 4){
		armed1 = 0;
		armed2 = 0;
		armed3 = 0;
		armed4 = 0;
		btnled_set_value(hmi_client, HMI_LED_1, HMI_VALUE_OFF);
		btnled_set_value(hmi_client, HMI_LED_2, HMI_VALUE_OFF);
		btnled_set_value(hmi_client, HMI_LED_3, HMI_VALUE_OFF);
		btnled_set_value(hmi_client, HMI_LED_4, HMI_VALUE_OFF);
		s1enset = 0;
		s1diset = 0;
		s2enset = 0;
		s2diset = 0;
		s3enset = 0;
		s3diset = 0;
		s4enset = 0;
		s4diset = 0;
	}
	else {
		armed1 = 1;
		armed2 = 1;
		armed3 = 1;
		armed4 = 1;
		btnled_set_value(hmi_client, HMI_LED_1, HMI_VALUE_ON);
		btnled_set_value(hmi_client, HMI_LED_2, HMI_VALUE_ON);
		btnled_set_value(hmi_client, HMI_LED_3, HMI_VALUE_ON);
		btnled_set_value(hmi_client, HMI_LED_4, HMI_VALUE_ON);
	}
}

void Main_task(uint_32 initial_data)
{
	hmi_client = _bsp_btnled_init();

	rtcs_init();

	_io_tfs_install("tfs:", static_data);

	http_server = httpd_server_init_af(http_root_dir, "\\index.html", AF_INET);

	HTTPD_SET_PARAM_CGI_TBL(http_server, http_cgi_params);

	httpd_server_run(http_server);

	//turn off LEDS
	btnled_set_value(hmi_client, HMI_LED_1, HMI_VALUE_OFF);
	btnled_set_value(hmi_client, HMI_LED_2, HMI_VALUE_OFF);
	btnled_set_value(hmi_client, HMI_LED_3, HMI_VALUE_OFF);
	btnled_set_value(hmi_client, HMI_LED_4, HMI_VALUE_OFF);
	
	//initialise alarm to off
	armed1 = 0;
	armed2 = 0;
	armed3 = 0;
	armed4 = 0;

	btnled_add_clb(hmi_client, HMI_BUTTON_1, HMI_VALUE_PUSH, button1_push, NULL);
	btnled_add_clb(hmi_client, HMI_BUTTON_2, HMI_VALUE_PUSH, button2_push, NULL);
	btnled_add_clb(hmi_client, HMI_BUTTON_3, HMI_VALUE_PUSH, button3_push, NULL);
	btnled_add_clb(hmi_client, HMI_BUTTON_4, HMI_VALUE_PUSH, button4_push, NULL);
	btnled_add_clb(hmi_client, HMI_BUTTON_5, HMI_VALUE_PUSH, button5_push, NULL);
	btnled_add_clb(hmi_client, HMI_BUTTON_6, HMI_VALUE_PUSH, button6_push, NULL);

	while(1){
		btnled_poll(hmi_client);
		ipcfg_task_poll();
		count++;
		if (count % 75 == 0){
			if (tripped1 == 1){btnled_toogle(hmi_client, HMI_LED_1);}
			if (tripped2 == 1){btnled_toogle(hmi_client, HMI_LED_2);}
			if (tripped3 == 1){btnled_toogle(hmi_client, HMI_LED_3);}
			if (tripped4 == 1){btnled_toogle(hmi_client, HMI_LED_4);}
		}
		_rtc_get_time(&curr_time);
		seconds = curr_time.seconds;	
		if (s1enset){
			if (s1diset){
				//if sensor 1 enable and disable times have been set then arm it
				if ((s1en.seconds < seconds) && (seconds < s1di.seconds)){
					armed1 = 1;
					btnled_set_value(hmi_client, HMI_LED_1, HMI_VALUE_ON);
				}
			}
		}
		if (s2enset){
			if (s2diset){
				//if sensor 2 enable and disable times have been set then arm it
				if ((s2en.seconds < seconds) && (seconds < s2di.seconds)){
					armed2 = 1;
					btnled_set_value(hmi_client, HMI_LED_2, HMI_VALUE_ON);
				}

			}
		}
		if (s3enset){
			if (s3diset){
				//if sensor 3 enable and disable times have been set then arm it
				if ((s3en.seconds < seconds) && (seconds < s3di.seconds)){
					armed3 = 1;
					btnled_set_value(hmi_client, HMI_LED_3, HMI_VALUE_ON);
				}

			}
		}
		if (s4enset){
			if (s4diset){
				//if sensor 4 enable and disable times have been set then arm it
				if ((s4en.seconds < seconds) && (seconds < s4di.seconds)){
					armed4 = 1;
					btnled_set_value(hmi_client, HMI_LED_4, HMI_VALUE_ON);
				}

			}
		}
	}
}
/* EOF */

