/**
* @Author: tqkcva@gmail.com
* @Description: 
* @History: April 06, tqkieu, file creation
*
**/
#ifndef at_serial_h_
#define at_serial_h_

int at_serial_init(int baudrate);
int at_serial_send_cmd(char * cmd);
int at_serial_send_data(char * data, int data_len); // for raw data sending
int at_serial_readline(char * buffer, int buffer_len);
int at_serial_waitline(char * buffer, int buffer_size, const char * str, unsigned long timeout);
int at_serial_polling(void);

#endif
