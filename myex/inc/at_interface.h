/**
* @Author: tqkcva@gmail.com
* @Description: 
* @History: April 06, tqkieu, file creation
*
**/
#ifndef at_interface_h_
#define at_interface_h_

struct GPRS_Profile
{
	int pdp_contex;
	char apn[16];
	char user[8];
	char pass[8];
};
struct Publish_Server
{
	char address[64];
	int port;
};

/* public variables for configuration */
extern struct GPRS_Profile at_int_gprs_profile;
extern struct Publish_Server at_int_server;

/* General functions */
int at_factory_reset(void);
int at_reset(void);
int at_enter_sim_pin(char * sim_pin);
int at_check_sim_pin(void); /* 1: SIM PIN, 2: SIM READY, 3: PUK REQUIRED */
int at_check_network(int retry_count);

/* SMS */
int at_sms_init(void); /* initiate SMS setting: SM memory, text mode, URC enabled */
int at_sms_send(char * sms, char * des_number);
int at_sms_write(char * sms, char * des_number); /* resturn stored index */
int at_sms_send_stored(int sms_index);
int at_sms_incoming(void); /* return incoming SMS index */
int at_sms_delete(int index); /* input -1 for delete all */

/* GPRS */
int at_gprs_init(int pdp_context, char * apn, char * user, char * passwd);
int at_gprs_release(int pdp_context);

/* TCP */
int at_tcp_connect(char * server, int port);
int at_tcp_send(char * data, int data_len);
int at_tcp_recv(char * buffer, int buffer_len);
int at_tcp_close(void);

/* UDP */
int at_udp_connect(char * server, int port);
int at_udp_send(char * data, int data_len);
int at_udp_recv(char * buffer, int buffer_len);
int at_udp_close(void);

/** GNSS **/
int at_gnss_start(void);
int at_gnss_process(void);
int at_gnss_publish_location(void);

#endif
