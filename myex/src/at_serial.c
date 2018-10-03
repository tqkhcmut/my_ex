/**
* @Author: tqkcva@gmail.com
* @Description: 
* @History: April 06, tqkieu, file creation
*
**/
#include "at_serial.h"
#include "usart.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define STM32F107RB_INITIALIZATION 1

#ifdef STM32F107RB_INITIALIZATION
#include "stm32f10x.h"
#include <string.h>
//
//#ifndef BYTE_FORMAT
//#define BYTE_FORMAT 1
//typedef enum 
//{
//	BIN = 0,
//	OCT,
//	DEC,
//	HEX
//} BYTE_FORMAT;
//#endif

#define UART4_BUFF_SIZE 512
char uart4_rx_buff[UART4_BUFF_SIZE];
int uart4_rx_len;

//extern uint32_t SystemCoreClock	= SYSCLK_FREQ_72MHz;
unsigned int break_time = 100; // default: 100 us

void UART4_init(int baudrate)
{
	// USART init
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	
#ifdef DEBUG
	USART1_sendStr("\nUART4_init");
#endif
	
	/* Configure the NVIC Preemption Priority Bits */  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	
	/* Enable the USARTy Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	/* GPIOC Periph clock enable */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
	/* TIM2 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	
	/* Configure PC10 and PC11 in output pushpull mode */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	USART_InitStructure.USART_BaudRate            = baudrate;
	USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits            = USART_StopBits_1;
	USART_InitStructure.USART_Parity              = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
	
	USART_Init(UART4, &USART_InitStructure);
	
	/* Enable UART4 Receive and Transmit interrupts */
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
	
	USART_Cmd(UART4, ENABLE);
	
	// calculate break_time
	break_time = 64000000/baudrate;
	
	// init timer base
	
	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = 65535;
	TIM_TimeBaseStructure.TIM_Prescaler = 64;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	/* TIM2 enable counter */
	TIM_Cmd(TIM2, ENABLE);
	
#ifdef DEBUG
	USART1_sendStr("\tDone");
#endif
	
}

void UART4_IRQHandler(void)
{
	if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)
	{
		/* Read one byte from the receive data register */
		uart4_rx_buff[uart4_rx_len++] = USART_ReceiveData(UART4);
		if (uart4_rx_len >= UART4_BUFF_SIZE)
		{
			uart4_rx_len = 0;
		}
		
		//    if(RxCounter1 == NbrOfDataToRead1)
		//    {
		//      /* Disable the USARTy Receive interrupt */
		//      USART_ITConfig(USARTy, USART_IT_RXNE, DISABLE);
		//    }
		TIM_SetCounter(TIM2, 0);
	}
}

int UART4_Available(void)
{
	if (TIM_GetCounter(TIM2) > break_time)
		return uart4_rx_len;
	else
		return 0;
}

int UART4_GetData(char * buffer, int len)
{
	if (len > uart4_rx_len)
		len = uart4_rx_len;
	USART_ITConfig(UART4, USART_IT_RXNE, DISABLE);
	for (len = 0; len < uart4_rx_len; len++)
	{
		buffer[len] = uart4_rx_buff[len];
	}
	uart4_rx_len = 0;
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
	return len;
}

void UART4_Flush(void)
{
	uart4_rx_len = 0;
}

void UART4_sendChar(char c)
{
	USART_SendData(UART4, c);
	while (USART_GetFlagStatus(UART4, USART_FLAG_TC) == RESET)
	{}
}

void UART4_sendStr(char Str[])
{
	while(*Str)
	{
		USART_SendData(UART4, *Str++);
		while (USART_GetFlagStatus(UART4, USART_FLAG_TC) == RESET)
		{}
	}
}

void UART4_sendNum(int num)
{
	int tmp = 10000000;
	if (num == 0)
	{
		UART4_sendChar('0');
		return;
	}
	if (num < 0)
	{
		UART4_sendChar('-');
		num = -num;
	}
	while (tmp > 0)
	{
		if (tmp <= num)
		{
			UART4_sendChar((num/tmp)%10 + '0');
		}
		tmp = tmp / 10;
	}
}

void UART4_sendFloat(float num)
{
	int __int = (int) num;
	UART4_sendNum(__int);
	UART4_sendChar('.');
	__int = (int)((num-__int)*100);
	if (__int < 0)
		__int = 0;
	UART4_sendNum(__int);
}

void UART4_sendByte(uint8_t b, BYTE_FORMAT f)
{
	uint8_t bitMask = 1;
	uint8_t i;
	switch (f)
	{
	case BIN:
		for (i = 8; i > 0; i--)
		{
			UART4_sendChar((b&(bitMask << i)) ? '1' : '0');
		}
		break;
	case OCT:
		break;
	case DEC:
		UART4_sendNum(b);
		break;
	case HEX:
		if(b/16 < 10){
			UART4_sendChar(0x30 + b/16);
		}else{
			UART4_sendChar(0x37 + b/16);
		}
		
		if(b%16 < 10){
			UART4_sendChar(0x30 + b%16);
		}else{
			UART4_sendChar(0x37 + b%16);
		}
		break;
	default:
		break;
	}
}
#endif // STM32F107RB_INITIALIZATION

char polling_buffer[UART4_BUFF_SIZE];

int at_serial_init(int baudrate)
{
	memset(polling_buffer, 0, UART4_BUFF_SIZE);
#ifdef STM32F107RB_INITIALIZATION
	UART4_init(baudrate);
#endif // STM32F107RB_INITIALIZATION
	return 0;
}
int at_serial_send_cmd(char * cmd)
{
	// debug
	USART1_sendStr("AT SEND: ");
	USART1_sendStr(cmd);
	USART1_sendStr("\r\n");
	
#ifdef STM32F107RB_INITIALIZATION
	UART4_sendStr(cmd);
	UART4_sendChar('\r');
#endif // STM32F107RB_INITIALIZATION
	// 100ms delay here
	vTaskDelay(100);
	return 0;
}
int at_serial_send_data(char * data, int data_len)
{
	// debug
	USART1_sendStr("AT SEND DATA: ");
	
#ifdef STM32F107RB_INITIALIZATION
	int i = 0;
	for (i = 0; i < data_len; i++)
	{
		UART4_sendChar(data[i]);
		// debug
		USART1_sendChar(data[i]);
	}
#endif // STM32F107RB_INITIALIZATION
	
	USART1_sendStr("\r\n");
	
	return 0;
}
int at_serial_readline(char * buffer, int buffer_len)
{
	int retry = 1000;
	/** debug **/
//	USART1_sendStr("at_serial_readline: ");
	/** end debug **/
	while (retry > 0)
	{
#ifdef STM32F107RB_INITIALIZATION
		if (uart4_rx_len)
		{
			int bytes_copied = 0;
			int i;
			for (i = 0; i < uart4_rx_len; i++)
			{
				if (uart4_rx_buff[i] != '\r' && uart4_rx_buff[i] != '\n')
				{
					// let's copy
					buffer[bytes_copied] = uart4_rx_buff[i];
					bytes_copied++;
				}
				else
				{
					if (bytes_copied == 0)
					{
						// nothing to do here, we need to find the first character of the line
					}
					else
					{
						// now it's time to end, return number bytes that copied
						for (i = 0; i < uart4_rx_len - bytes_copied; i++)
						{
							uart4_rx_buff[i] = uart4_rx_buff[bytes_copied + i];
						}
						uart4_rx_len = uart4_rx_len - bytes_copied;
						if (uart4_rx_len < 0)
						{
							uart4_rx_len = 0;
						}
						return bytes_copied;
					}
				}
			}
		}
#endif // STM32F107RB_INITIALIZATION
		retry--;
		vTaskDelay(1);
	}
//	USART1_sendStr("timeout\r\n");
	return 0;
}

int at_serial_polling(void)
{
	if (UART4_Available() > 0)
	{
		int read_len = UART4_Available();
		UART4_GetData(polling_buffer, read_len);
		polling_buffer[read_len] = '\0';
		/* TODO: Process URC here */
		USART1_sendStr(polling_buffer);
	}
	return 0;
}

//int read_len = 0;
//char _buff[256];
//
//int at_serial_waitline(const char * str, unsigned long timeout)
//{
//	int res = -1;
//	unsigned long start_time = 0;
//	start_time = xTaskGetTickCount();
//
//	/** debug **/
//	USART1_sendStr("at_serial_waitline: ");
//	USART1_sendStr((char *)str);
//	USART1_sendStr(" : ");
//	USART1_sendNum(timeout);
//	USART1_sendStr("\r\n");
//	
//	while(xTaskGetTickCount() - start_time < timeout)
//	{
//		read_len = at_serial_readline(_buff, 127);
//		_buff[read_len] = 0;
//		if(read_len > 0)
//		{
//			USART1_sendStr("Received line: ");
//			USART1_sendStr(_buff);
//			USART1_sendStr("\r\n");
//			if (memcmp(str, _buff, strlen(str)) == 0)
//			{
//				res = 0;
//				break;
//			}
//		}
//		vTaskDelay(1);
//	}
//	
//	/** debug **/
//	if (res == 0)
//	{
//		USART1_sendStr("Found with time: ");
//		USART1_sendNum(xTaskGetTickCount() - start_time);
//		USART1_sendStr("\r\n");
//	}
//	else
//	{
//		USART1_sendStr("Timeout. Received: \"");
//		USART1_sendStr(_buff);
//		USART1_sendStr("\"\r\n");
//	}
//	
//	return res;
//}

int at_serial_waitline(char * buffer, int buffer_size, const char * str, unsigned long timeout)
{
	int res = -1;
	int read_len = 0;
	unsigned long start_time = 0;
	start_time = xTaskGetTickCount();

	/** debug **/
	USART1_sendStr("at_serial_waitline: ");
	USART1_sendStr((char *)str);
	USART1_sendStr(" : ");
	USART1_sendNum(timeout);
	USART1_sendStr("\r\n");
	
	while(xTaskGetTickCount() - start_time < timeout)
	{
		memset(buffer, 0, buffer_size);
		read_len = at_serial_readline(buffer, buffer_size-1);
		if(read_len > 0)
		{
			USART1_sendStr("Received line: ");
			USART1_sendStr(buffer);
			USART1_sendStr("\r\n");
			if (memcmp(str, buffer, strlen(str)) == 0)
			{
				res = 0;
				break;
			}
		}
		vTaskDelay(1);
	}
	
	/** debug **/
	if (res == 0)
	{
		USART1_sendStr("Found with time: ");
		USART1_sendNum(xTaskGetTickCount() - start_time);
		USART1_sendStr("\r\n");
		return read_len;
	}
	else
	{
		USART1_sendStr("Timeout.\r\n");
	}
	
	return res;
}