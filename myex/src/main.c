#include "stm32f10x.h"

/* Standard includes. */
//#include <stdio.h>
#include <string.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Library includes. */
#include "stm32f10x_it.h"

#include "usart.h"
#include "at_serial.h"
#include "at_interface.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
GPIO_InitTypeDef GPIO_InitStructure;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
void delay_ms(uint32_t nCount);

#define MAIN_TASK_PRIO   ( tskIDLE_PRIORITY + 1 )
#define USART_TASK_PRIO   ( tskIDLE_PRIORITY + 1 )
#define LED_TASK_PRIO   ( tskIDLE_PRIORITY + 1 )
#define MYEX_TASK_PRIO   ( tskIDLE_PRIORITY + 1 )

void LED_Task(void * params)
{
	/* GPIOD Periph clock enable */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	/* Configure PD0 and PD2 in output pushpull mode */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	for(;;)
	{
		GPIO_SetBits(GPIOB, GPIO_Pin_5);
		delay_ms(1000);
		GPIO_ResetBits(GPIOB, GPIO_Pin_5);
		delay_ms(1000);
	}
}

void test_at(void * params)
{
	char line[128];
	USART1_sendStr("UART debug task: begin");
	at_serial_init(115200);
	line[0] = '\0';
	int line_len = 0;
	for(;;)
	{
		line_len = USART1_readline(line, 127);
		if (line_len > 0)
		{			
			line[line_len] = '\0';
			USART1_sendStr(line); /* debug */
			USART1_sendStr("\r\n"); /* debug */
			at_serial_send_cmd(line);
		}
		else
		{
			vTaskDelay(1);
		}
		at_serial_polling();
	}
}

void myex_task(void * params)
{
	// @TODO: reload configuration from flash here

	USART1_sendStr("myex_task: begin\r\n");
	at_serial_init(115200);
	USART1_sendStr("myex_task: delaying for SIM900 module start up\r\n");
	vTaskDelay(20000);
	USART1_sendStr("myex_task: end delay\r\n");
	at_factory_reset();
	USART1_sendStr("myex_task: delaying for SIM900 module start up\r\n");
	vTaskDelay(20000);
	USART1_sendStr("myex_task: end delay\r\n");
	at_gprs_init(at_int_gprs_profile.pdp_contex, at_int_gprs_profile.apn, at_int_gprs_profile.user, at_int_gprs_profile.pass);
	at_gnss_start();
	for(;;)
	{
		at_gnss_process();
		vTaskDelay(1);
	}
}

void gnss_test_task(void * params)
{
	char __location_buff[256];
	USART1_sendStr("myex_task: begin\r\n");
	at_serial_init(115200);
	USART1_sendStr("myex_task: delaying for SIM900 module start up\r\n");
	vTaskDelay(20000);
	at_gnss_start();
	for(;;)
	{
		at_serial_send_cmd("AT+CGNSINF");
		at_serial_waitline(__location_buff, 256, "+CGNSINF: ", 10000);
		USART1_sendStr("+CGNSINF Received: ");
		USART1_sendStr(__location_buff);
		USART1_sendStr("\r\n");
		at_serial_waitline(__location_buff, 256, "OK", 10000);

		vTaskDelay(1000);
	}
}

void main_task(void * params)
{
	/* Driver initialization */
	USART1_init();
	USART1_sendStr("UART debug task: initialized\r\n");

	xTaskCreate(LED_Task, "LED", configMINIMAL_STACK_SIZE * 2, NULL, LED_TASK_PRIO, NULL);
	//	xTaskCreate(test_at, "at", configMINIMAL_STACK_SIZE * 2, NULL, USART_TASK_PRIO, NULL);
	//	xTaskCreate(myex_task, "myex", configMINIMAL_STACK_SIZE * 2, NULL, MYEX_TASK_PRIO, NULL);
	xTaskCreate(gnss_test_task, "gnss test task", configMINIMAL_STACK_SIZE * 2, NULL, MYEX_TASK_PRIO, NULL);

	for( ;; )
	{
		vTaskDelete(NULL); // exit by itself
	}
}

int main()
{	
	// start main task
	xTaskCreate(main_task, "main", configMINIMAL_STACK_SIZE * 2, NULL, MAIN_TASK_PRIO, NULL);

	/* Start the scheduler. */
	vTaskStartScheduler();

	for(;;); //trap the cpu, but never get there
}

//*****************************************************************************
//
// This hook is called by FreeRTOS when an stack overflow error is detected.
//
//*****************************************************************************
void
vApplicationStackOverflowHook(xTaskHandle *pxTask, signed char *pcTaskName)
{
	//
	// This function can not return, so loop forever.  Interrupts are disabled
	// on entry to this function, so no processor interrupts will interrupt
	// this loop.
	//
	while(1)
	{
	}
}

/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
	// free
}

void vApplicationMallocFailedHook( void )
{
	// if you are enter this function, it mean memory allocation 
	// have been failed.
	// This cause may be you have not enough heap memory. Expand
	// FreeRTOS memory on FreeRTOSConfig.h may fix this broblem.

	for(;;); // halt the CPU
}

void delay_ms(uint32_t nCount)
{
	vTaskDelay(nCount);
}

#ifdef  DEBUG
/* Keep the linker happy. */
void assert_failed( unsigned char* pcFile, unsigned long ulLine )
{
	for( ;; )
	{
	}
}
#endif
