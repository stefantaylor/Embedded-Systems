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

HMI_CLIENT_STRUCT_PTR hmi_client;
//hmi_client = _bsp_btnled_init();

int count = 0;
int armed;
int tripped1;
int tripped2;
int tripped3;
int tripped4;

void button1_push(void *ptr){
	//trips alarm 1
	if (armed){
		tripped1 = 1;
		btnled_set_value(hmi_client, HMI_LED_1, HMI_VALUE_OFF);
	}
}


void button2_push(void *ptr){
	//trips alarm 2
	if (armed){	
		tripped2 = 1;
		btnled_set_value(hmi_client, HMI_LED_2, HMI_VALUE_OFF);
	}
}

void button3_push(void *ptr){
	//trips alarm 3
	if (armed){
		tripped3 = 1;
		btnled_set_value(hmi_client, HMI_LED_3, HMI_VALUE_OFF);
	}
}

void button4_push(void *ptr){
	//trips alarm 4
	if (armed){
		tripped4 = 1;
		btnled_set_value(hmi_client, HMI_LED_4, HMI_VALUE_OFF);
	}
}

void button5_push(void *ptr){
	//resets any tripped alarms
	if (armed){
		tripped1 = 0;
		tripped2 = 0;
		tripped3 = 0;
		tripped4 = 0;
		btnled_set_value(hmi_client, HMI_LED_1, HMI_VALUE_ON);
		btnled_set_value(hmi_client, HMI_LED_2, HMI_VALUE_ON);
		btnled_set_value(hmi_client, HMI_LED_3, HMI_VALUE_ON);
		btnled_set_value(hmi_client, HMI_LED_4, HMI_VALUE_ON);
	}

}

void button6_push(void *ptr){
	//arms/disarms the alarm system
	if (armed){
		armed = 0;
		btnled_set_value(hmi_client, HMI_LED_1, HMI_VALUE_OFF);
		btnled_set_value(hmi_client, HMI_LED_2, HMI_VALUE_OFF);
		btnled_set_value(hmi_client, HMI_LED_3, HMI_VALUE_OFF);
		btnled_set_value(hmi_client, HMI_LED_4, HMI_VALUE_OFF);
	}
	else {
		armed = 1;
		btnled_set_value(hmi_client, HMI_LED_1, HMI_VALUE_ON);
		btnled_set_value(hmi_client, HMI_LED_2, HMI_VALUE_ON);
		btnled_set_value(hmi_client, HMI_LED_3, HMI_VALUE_ON);
		btnled_set_value(hmi_client, HMI_LED_4, HMI_VALUE_ON);
	}
}

/*void button4_release(void *ptr){
btnled_set_value
(hmi_client, HMI_LED_4, HMI_VALUE_OFF);
}*/

void Main_task(uint_32 initial_data)
{
	hmi_client = _bsp_btnled_init();
	//turn off LEDS
	btnled_set_value(hmi_client, HMI_LED_1, HMI_VALUE_OFF);
	btnled_set_value(hmi_client, HMI_LED_2, HMI_VALUE_OFF);
	btnled_set_value(hmi_client, HMI_LED_3, HMI_VALUE_OFF);
	btnled_set_value(hmi_client, HMI_LED_4, HMI_VALUE_OFF);
	
	//initialise alarm to off
	armed = 0;

	btnled_add_clb(hmi_client, HMI_BUTTON_1, HMI_VALUE_PUSH, button1_push, NULL);
	btnled_add_clb(hmi_client, HMI_BUTTON_2, HMI_VALUE_PUSH, button2_push, NULL);
	btnled_add_clb(hmi_client, HMI_BUTTON_3, HMI_VALUE_PUSH, button3_push, NULL);
	btnled_add_clb(hmi_client, HMI_BUTTON_4, HMI_VALUE_PUSH, button4_push, NULL);
	btnled_add_clb(hmi_client, HMI_BUTTON_5, HMI_VALUE_PUSH, button5_push, NULL);
	btnled_add_clb(hmi_client, HMI_BUTTON_6, HMI_VALUE_PUSH, button6_push, NULL);

//	btnled_add_clb(hmi_client, HMI_BUTTON_4, HMI_VALUE_RELEASE, button4_release, NULL);

	while(1){
		btnled_poll(hmi_client);
		count++;
		if (count % 50 == 0){
			if (tripped1 == 1){btnled_toogle(hmi_client, HMI_LED_1);}
			if (tripped2 == 1){btnled_toogle(hmi_client, HMI_LED_2);}
			if (tripped3 == 1){btnled_toogle(hmi_client, HMI_LED_3);}
			if (tripped4 == 1){btnled_toogle(hmi_client, HMI_LED_4);}
		}
	}
}
/* EOF */

