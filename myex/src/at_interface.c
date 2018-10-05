/**
* @Author: tqkcva@gmail.com
* @Description: 
* @History: April 06, tqkieu, file creation
*
**/

/** global variable **/
#define BUFFER_SIZE 256
char __buffer[BUFFER_SIZE];
char __sprintf_buff[BUFFER_SIZE];
char __location_buff[BUFFER_SIZE];
int __read_len = 0;
	
/** interface include **/
#include "at_interface.h"
#include "at_serial.h"
#include "usart.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* enhance command processing */
#include "c_comm.h"

/* public variables for configuration */
struct GPRS_Profile at_int_gprs_profile =
{
		1,
		"v-internet",
		"",
		""
};

struct Publish_Server at_int_server =
{
		"swvalidation-pub02.tma.com.vn",
		7038
};

/* General functions */
int at_factory_reset(void)
{
	at_serial_send_cmd("AT&F");
	at_serial_waitline(__buffer, BUFFER_SIZE, "OK", 3000);
	at_reset();
	return 0;
}
int at_reset(void)
{
	at_serial_send_cmd("AT+CFUN=1,1");
	at_serial_waitline(__buffer, BUFFER_SIZE, "OK", 3000);
	// need a short delay here
	return 0;
}
int at_enter_sim_pin(char * sim_pin)
{
	int res = at_check_sim_pin();
	switch (res)
	{
	case 1: /* SIM PIN REQUIRED */
		{
			memset(__sprintf_buff, 0, BUFFER_SIZE);
			sprintf(__sprintf_buff, "AT+CPIN=%s", sim_pin);
			at_serial_send_cmd(__sprintf_buff);
			at_serial_waitline(__buffer, BUFFER_SIZE, "OK", 3000);
			at_serial_waitline(__buffer, BUFFER_SIZE, "PBREADY", 30000);
			break;
		}
	case 2: /* SIM PIN READY */
		break;
	case 3: /* PUK REQUIRED */
		break;
	default:
		break;
	}
	return 0;
}
int at_check_sim_pin(void) /* 1: SIM PIN, 2: SIM READY, 3: PUK REQUIRED */
{
	int res = 0;
	at_serial_send_cmd("ATE0");
	at_serial_waitline(__buffer, BUFFER_SIZE, "OK", 3000);
	at_serial_send_cmd("AT+CPIN?");
	at_serial_waitline(__buffer, BUFFER_SIZE, "+CPIN: ", 3000);
	if (memcmp(__buffer, "+CPIN: SIM PIN", 14) == 0)
	{
		res = 1;
	}
	if (memcmp(__buffer, "+CPIN: READY", 12) == 0)
	{
		res = 2;
	}
	if (memcmp(__buffer, "+CPIN: SIM PUK", 14) == 0)
	{
		res = 3;
	}
	at_serial_waitline(__buffer, BUFFER_SIZE, "OK", 3000);
	return res;
}
int at_check_network(int retry_count)
{
	int i = 0, res = 0;
	at_serial_send_cmd("ATE0");
	at_serial_waitline(__buffer, BUFFER_SIZE, "OK", 3000);
	at_serial_send_cmd("AT+CMEE=1");
	at_serial_waitline(__buffer, BUFFER_SIZE, "OK", 3000);
	at_serial_send_cmd("AT+CREG=0");
	at_serial_waitline(__buffer, BUFFER_SIZE, "OK", 3000);
	for (i = 0; i < retry_count; i++)
	{
		at_serial_send_cmd("AT+CREG?");
		if (at_serial_waitline(__buffer, BUFFER_SIZE, "+CREG: 0,1", 4000) > 0)
		{
			res = 1;
			at_serial_waitline(__buffer, BUFFER_SIZE, "OK", 3000);
			break;
		}
		at_serial_waitline(__buffer, BUFFER_SIZE, "OK", 3000);
		vTaskDelay(3000);
	}
	return res;
}

/* SMS */
int at_sms_init(void) /* initiate SMS setting: SM memory, text mode, URC enabled */
{
	at_serial_send_cmd("ATE0");
	at_serial_waitline(__buffer, BUFFER_SIZE, "OK", 3000);
	at_serial_send_cmd("AT+CMEE=1");
	at_serial_waitline(__buffer, BUFFER_SIZE, "OK", 3000);
	at_serial_send_cmd("AT+CNMI=1,1");
	at_serial_waitline(__buffer, BUFFER_SIZE, "OK", 3000);
	at_serial_send_cmd("AT+CMGF=1");
	at_serial_waitline(__buffer, BUFFER_SIZE, "OK", 3000);
	at_serial_send_cmd("AT+CPMS=\"SM\",\"SM\",\"SM\"");
	at_serial_waitline(__buffer, BUFFER_SIZE, "OK", 3000);
	at_serial_send_cmd("AT+CMGD=1,4");
	at_serial_waitline(__buffer, BUFFER_SIZE, "OK", 30000);	
	return 0;
}
int at_sms_send(char * sms, char * des_number)
{
	memset(__sprintf_buff, 0, BUFFER_SIZE);
	sprintf(__sprintf_buff, "AT+CMGS=%s", des_number);
	at_serial_send_cmd(__sprintf_buff);
	at_serial_waitline(__buffer, BUFFER_SIZE, ">", 3000);
	memset(__sprintf_buff, 0, BUFFER_SIZE);
	sprintf(__sprintf_buff, "%s\x1A", des_number);
	at_serial_send_cmd(__sprintf_buff);
	at_serial_waitline(__buffer, BUFFER_SIZE, "OK", 10000);	
	return 0;
}
int at_sms_write(char * sms, char * des_number) /* return stored index */
{
	memset(__sprintf_buff, 0, BUFFER_SIZE);
	sprintf(__sprintf_buff, "AT+CMGW=%s", des_number);
	at_serial_send_cmd(__sprintf_buff);
	at_serial_waitline(__buffer, BUFFER_SIZE, ">", 3000);
	memset(__sprintf_buff, 0, BUFFER_SIZE);
	sprintf(__sprintf_buff, "%s\x1A", des_number);
	at_serial_send_cmd(__sprintf_buff);
	at_serial_waitline(__buffer, BUFFER_SIZE, "OK", 10000);	
	return 0;
}
int at_sms_send_stored(int sms_index)
{
	memset(__sprintf_buff, 0, BUFFER_SIZE);
	sprintf(__sprintf_buff, "AT+CMSS=%d", sms_index);
	at_serial_send_cmd(__sprintf_buff);
	at_serial_waitline(__buffer, BUFFER_SIZE, "OK", 10000);	
	return 0;
}
int at_sms_incoming(void) /* return incoming SMS index */
{
	return 0;
}
int at_sms_delete(int index) /* input -1 for delete all */
{
	if (index == -1)
	{
		at_serial_send_cmd("AT+CMGD=1,4");
		at_serial_waitline(__buffer, BUFFER_SIZE, "OK", 10000);	
	}
	else
	{
		memset(__sprintf_buff, 0, BUFFER_SIZE);
		sprintf(__sprintf_buff, "AT+CMGD=%d,0", index);
		at_serial_send_cmd(__sprintf_buff);
		at_serial_waitline(__buffer, BUFFER_SIZE, "OK", 10000);	
	}
	return 0;
}

/* GPRS */
int at_gprs_init(int pdp_context, char * apn, char * user, char * passwd)
{
	at_serial_send_cmd("ATE0");
	at_serial_waitline(__buffer, BUFFER_SIZE, "OK", 3000);
	at_serial_send_cmd("AT+CMEE=1");
	at_serial_waitline(__buffer, BUFFER_SIZE, "OK", 3000);
	
	at_enter_sim_pin("3699");
	at_check_network(10);
	
	at_serial_send_cmd("AT+CGATT=1");
	at_serial_waitline(__buffer, BUFFER_SIZE, "OK", 10000);
	
	memset(__sprintf_buff, 0, BUFFER_SIZE);
	sprintf(__sprintf_buff, "AT+CGDCONT=%d,\"IP\",\"%s\"", pdp_context, apn);
	at_serial_send_cmd(__sprintf_buff);
	at_serial_waitline(__buffer, BUFFER_SIZE, "OK", 10000);	
	
	memset(__sprintf_buff, 0, BUFFER_SIZE);
	sprintf(__sprintf_buff, "AT+CGACT=1,%d", pdp_context);
	at_serial_send_cmd(__sprintf_buff);
	at_serial_waitline(__buffer, BUFFER_SIZE, "OK", 10000);
	
	return 0;
}
int at_gprs_release(int pdp_context)
{	
	memset(__sprintf_buff, 0, BUFFER_SIZE);
	sprintf(__sprintf_buff, "AT+CGACT=0,%d", pdp_context);
	at_serial_send_cmd(__sprintf_buff);
	at_serial_waitline(__buffer, BUFFER_SIZE, "OK", 10000);	
	
	memset(__sprintf_buff, 0, BUFFER_SIZE);
	sprintf(__sprintf_buff, "AT+CGDCONT=%d", pdp_context);
	at_serial_send_cmd(__sprintf_buff);
	at_serial_waitline(__buffer, BUFFER_SIZE, "OK", 10000);	
	
	return 0;
}

/* TCP */
int at_tcp_connect(char * server, int port)
{
	/** TCP initialization **/
	memset(__sprintf_buff, 0, BUFFER_SIZE);
	sprintf(__sprintf_buff, "AT+CIPMUX=0");
	at_serial_send_cmd(__sprintf_buff);
	at_serial_waitline(__buffer, BUFFER_SIZE, "OK", 10000);	
	
	memset(__sprintf_buff, 0, BUFFER_SIZE);
	sprintf(__sprintf_buff, "AT+CIPQSEND=0");
	at_serial_send_cmd(__sprintf_buff);
	at_serial_waitline(__buffer, BUFFER_SIZE, "OK", 10000);	
	/** END TCP initialization **/
	
	memset(__sprintf_buff, 0, BUFFER_SIZE);
	sprintf(__sprintf_buff, "AT+CIPSTART=\"TCP\",\"%s\",\"%d\"", server, port);
	at_serial_send_cmd(__sprintf_buff);
	at_serial_waitline(__buffer, BUFFER_SIZE, "OK", 10000);	
	at_serial_waitline(__buffer, BUFFER_SIZE, "CONNECT OK", 80000);	
	
	return 0;
}

int at_tcp_send(char * data, int data_len)
{	
	memset(__sprintf_buff, 0, BUFFER_SIZE);
	sprintf(__sprintf_buff, "AT+CIPSEND=%d", data_len);
	at_serial_send_cmd(__sprintf_buff);
	at_serial_waitline(__buffer, BUFFER_SIZE, ">", 10000);	
	
	at_serial_send_data(data, data_len);	
	if (at_serial_waitline(__buffer, BUFFER_SIZE, "SEND OK", 30000) < 0)
	{
		return 1;
	}	
	return 0;
}
int at_tcp_recv(char * buffer, int buffer_len)
{
	/** TODO: pending at this time **/
	return 0;
}
int at_tcp_close(void)
{
	at_serial_send_cmd("AT+CIPCLOSE");
	if (at_serial_waitline(__buffer, BUFFER_SIZE, "CLOSE OK", 10000) < 0)
	{
		return 1;
	}
	
	return 0;
}

/* UDP */
int at_udp_connect(char * server, int port)
{
	return 0;
}
int at_udp_send(char * data, int data_len)
{
	return 0;
}
int at_udp_recv(char * buffer, int buffer_len)
{
	return 0;
}
int at_udp_close(void)
{
	return 0;
}

/** GNSS **/
int at_gnss_start(void)
{	
	at_serial_send_cmd("AAT");
	at_serial_waitline(__buffer, BUFFER_SIZE, "OK", 1000);
	at_serial_send_cmd("ATE0");
	at_serial_waitline(__buffer, BUFFER_SIZE, "OK", 1000);
	
	at_serial_send_cmd("AT+CGNSPWR?");
	if (at_serial_waitline(__buffer, BUFFER_SIZE, "+CGNSPWR: 1", 4000) > 0)
	{
		at_serial_waitline(__buffer, BUFFER_SIZE, "OK", 3000);
		USART1_sendStr("GNSS has been initialized\r\n");
	}
	else
	{
		at_serial_waitline(__buffer, BUFFER_SIZE, "OK", 3000);
		USART1_sendStr("GNSS has been turned off\r\nTurn it on:\r\n");
		at_serial_send_cmd("AT+CGNSPWR=1");
		at_serial_waitline(__buffer, BUFFER_SIZE, "GNSS POWER CONTROL ON", 10000);	
		at_serial_waitline(__buffer, BUFFER_SIZE, "OK", 10000);			
		USART1_sendStr("Done\r\n");
	}
	return 0;
}
int at_gnss_process(void)
{	
	static unsigned long last_update = 0;
		
	if (xTaskGetTickCount() - last_update > 60000)
	{
		last_update = xTaskGetTickCount();
		at_gnss_publish_location();
	}
	
	return 0;
}

int set_config(const char * key, const char * value)
{
	char flash_update = 0;

	if (memcmp(key, "server", 6) == 0)
	{

	}
	if (memcmp(key, "port", 6) == 0)
	{

	}
	if (memcmp(key, "pdp", 6) == 0)
	{

	}
	if (memcmp(key, "apn", 6) == 0)
	{

	}
	if (memcmp(key, "pass", 6) == 0)
	{

	}
	if (memcmp(key, "user", 6) == 0)
	{

	}

	if (flash_update == 1)
	{
		// @TODO: update to flash here
	}

	return 0;
}

struct c_comm configs[] =
{
		{"config", set_config},
		{NULL, NULL},
};

int at_cmds_init(void)
{
	return c_comm_register(configs);
}

/* Reconfiguration parameter:
config:server=sample.com\r
config:port=1234\r
config:pdp=1\r
config:apn=v-internet\r
config:user=\r
config:pass=\r
 *
 * Work with debug port only
 */
int at_reconfig_process(void)
{
	if (USART1_Available())
	{
		USART1_readline(__location_buff, BUFFER_SIZE - 1);
		c_comm_process(__location_buff);
	}

	/* @NOTE: no delaying here */

	return 0;
}

int at_gnss_publish_location(void)
{
	int res = 0;
	
	at_serial_send_cmd("AT+CGNSINF");
	__read_len = at_serial_waitline(__location_buff, BUFFER_SIZE, "+CGNSINF: ", 10000);
	at_serial_waitline(__buffer, BUFFER_SIZE, "OK", 10000);
	USART1_sendStr("+CGNSINF Received: ");	
	USART1_sendStr(__location_buff);
	USART1_sendStr("\r\n");
	
	if (at_tcp_send(__location_buff, __read_len) != 0)
	{
		at_tcp_close();
		at_tcp_connect(at_int_server.address, at_int_server.port);
		at_tcp_send(__location_buff, __read_len);
	}
	
	return res;
}
