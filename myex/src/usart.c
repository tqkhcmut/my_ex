#include "usart.h"
#include <string.h>

#define USART_BUFF_SIZE 128
unsigned char usart_rx_buff[USART_BUFF_SIZE];
unsigned char usart_rx_len;

void USART1_init(void)
{
  // USART init
  USART_InitTypeDef USART_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Configure the NVIC Preemption Priority Bits */  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
  
  /* Enable the USARTy Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  /* GPIOD Periph clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);
   
  /* Configure PA9 and PA10 in output pushpull mode */
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  USART_InitStructure.USART_BaudRate            = 115200;
  USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits            = USART_StopBits_1;
  USART_InitStructure.USART_Parity              = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
  
  USART_Init(USART1, &USART_InitStructure);
  
  /* Enable USART1 Receive and Transmit interrupts */
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
  
  USART_Cmd(USART1, ENABLE);
}


void USART1_IRQHandler(void)
{
  if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
  {
    /* Read one byte from the receive data register */
    usart_rx_buff[usart_rx_len++] = USART_ReceiveData(USART1);
    if (usart_rx_len >= USART_BUFF_SIZE)
    {
      usart_rx_len = 0;
    }

//    if(RxCounter1 == NbrOfDataToRead1)
//    {
//      /* Disable the USARTy Receive interrupt */
//      USART_ITConfig(USARTy, USART_IT_RXNE, DISABLE);
//    }
  }
}

int USART1_Available(void)
{
  return usart_rx_len;
}
int USART1_GetData(char * buffer, int len)
{
  memcpy(buffer, usart_rx_buff, len);
  return len;
}
void USART1_Flush(void)
{
  usart_rx_len = 0;
}

void USART1_sendChar(char c)
{
  USART_SendData(USART1, c);
  while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
  {}
}

void USART1_sendStr(char Str[])
{
  while(*Str)
  {
    USART_SendData(USART1, *Str++);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
    {}
  }
}

void USART1_sendNum(int num)
{
  int tmp = 10000000;
  if (num == 0)
  {
    USART1_sendChar('0');
    return;
  }
  if (num < 0)
  {
    USART1_sendChar('-');
    num = -num;
  }
  while (tmp > 0)
  {
    if (tmp <= num)
    {
      USART1_sendChar((num/tmp)%10 + '0');
    }
    tmp = tmp / 10;
  }
}

void USART1_sendFloat(float num)
{
  int __int = (int) num;
  USART1_sendNum(__int);
  USART1_sendChar('.');
  __int = (int)((num-__int)*100);
  if (__int < 0)
    __int = 0;
  USART1_sendNum(__int);
}

void USART1_sendByte(uint8_t b, BYTE_FORMAT f)
{
  uint8_t bitMask = 1;
  uint8_t i;
  switch (f)
  {
  case BIN:
    for (i = 8; i > 0; i--)
    {
      USART1_sendChar((b&(bitMask << i)) ? '1' : '0');
    }
    break;
  case OCT:
    break;
  case DEC:
    USART1_sendNum(b);
    break;
  case HEX:
    if(b/16 < 10){
      USART1_sendChar(0x30 + b/16);
    }else{
      USART1_sendChar(0x37 + b/16);
    }
    
    if(b%16 < 10){
      USART1_sendChar(0x30 + b%16);
    }else{
      USART1_sendChar(0x37 + b%16);
    }
    break;
  default:
    break;
  }
}

int USART1_readline(char * buffer, int buffer_len)
{
	if (USART1_Available())
	{
		int bytes_copied = 0;
		int i;
		for (i = 0; i < USART1_Available(); i++)
		{
			if (usart_rx_buff[i] != '\r' && usart_rx_buff[i] != '\n')
			{
				// let's copy
				buffer[bytes_copied] = usart_rx_buff[i];
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
					USART1_Flush();
					return bytes_copied;
				}
			}
		}
	}
	return 0;
}
