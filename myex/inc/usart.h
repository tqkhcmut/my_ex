/**
* @Author: tqkcva@gmail.com
* @Description: 
* @History: April 06, tqkieu, file creation
*
**/
#ifndef _usart_h_
#define _usart_h_

#ifdef __cplusplus
 extern "C" {
#endif
	 
//#define USE_USART

#include "stm32f10x.h"

#ifndef BYTE_FORMAT
typedef enum 
{
	BIN = 0,
	OCT,
	DEC,
	HEX
} BYTE_FORMAT;
#endif

//#define USART_BUFFER_SIZE 	32
//#define USART_BUFFERED

void USART1_init(void);
void USART1_sendChar(char c);
void USART1_sendStr(char Str[]);
void USART1_sendNum(int num);
void USART1_sendFloat(float num);
void USART1_sendByte(uint8_t b, BYTE_FORMAT f);
int USART1_Available(void);
int USART1_GetData(char * buffer, int len);
void USART1_Flush(void);
int USART1_readline(char * buffer, int buffer_len);

#ifdef __cplusplus
}
#endif

#endif

