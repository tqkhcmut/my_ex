#include "c_comm.h"

#define C_COMM_DEBUG 0

#ifdef C_COMM_DEBUG
//#include <stdio.h>
#include "usart.h"
#endif
#include <string.h>

char __cmd[COMMAND_STR_SIZE];
char __key[KEY_STR_SIZE];
char __val[VALUE_STR_SIZE];

struct c_comm * __register_list[REGISTER_SIZE];
int __register_count = 0;
int __register_ring = 0;

int c_comm_analyze(char * line)
{
	// line should have format bellow
	// command:key=value\r

	char * first, *last;

	memset(__cmd, 0, COMMAND_STR_SIZE);
	memset(__key, 0, KEY_STR_SIZE);
	memset(__val, 0, VALUE_STR_SIZE);

	first = line;
	last = strchr(line, ':');
	if (last == NULL || last - first >= COMMAND_STR_SIZE)
	{
#ifdef C_COMM_DEBUG
//		printf("\r\nERROR: Command input out of size\r\n");
		USART1_sendStr("\r\nERROR: Command input out of size\r\n");
#endif
		return -1;
	}
	else
	{
#ifdef C_COMM_DEBUG
//		printf("\r\nCopy command to buffer.\r\n");
		USART1_sendStr("\r\nCopy command to buffer.\r\n");
#endif
		memcpy(__cmd, first, last - first);
		first = last + 1;
	}

	last = strchr(line, '=');
	if (last == NULL || last - first >= KEY_STR_SIZE)
	{
#ifdef C_COMM_DEBUG
//		printf("\r\nERROR: Key input out of size\r\n");
		USART1_sendStr("\r\nERROR: Key input out of size\r\n");
#endif
		memset(__cmd, 0, COMMAND_STR_SIZE);
		return -1;
	}
	else
	{
#ifdef C_COMM_DEBUG
//		printf("\r\nCopy key to buffer.\r\n");
		USART1_sendStr("\r\nCopy key to buffer.\r\n");
#endif
		memcpy(__key, first, last - first);
		first = last + 1;
	}

	last = strchr(line, '\r');
	if (last == NULL || last - first >= VALUE_STR_SIZE)
	{
#ifdef C_COMM_DEBUG
//		printf("\r\nERROR: Value input out of size\r\n");
		USART1_sendStr("\r\nERROR: Value input out of size\r\n");
#endif
		memset(__cmd, 0, COMMAND_STR_SIZE);
		memset(__key, 0, KEY_STR_SIZE);
		return -1;
	}
	else
	{
#ifdef C_COMM_DEBUG
//		printf("\r\nCopy value to buffer.\r\n");
		USART1_sendStr("\r\nCopy value to buffer.\r\n");
#endif
		memcpy(__val, first, last - first);
		first = last + 1;
	}

#ifdef C_COMM_DEBUG
//	printf("\r\nCommand: %s\r\nKey: %s\r\nValue: %s\r\n", __cmd, __key, __val);
	USART1_sendStr("\r\nCommand: ");
	USART1_sendStr(__cmd);
	USART1_sendStr("\r\nKey: ");
	USART1_sendStr(__key);
	USART1_sendStr("\r\nValue: ");
	USART1_sendStr(__val);
	USART1_sendStr("\r\n");
#endif

	return 0;
}

int c_comm_register(struct c_comm cmd_list[])
{
	int i = 0;
	if (__register_count == 0)
	{
		for (i = 0; i < REGISTER_SIZE; i++)
		{
			__register_list[i] = NULL;
		}
	}
	i = 0;
	while (cmd_list[i].command != NULL)
	{
		if (strlen(cmd_list[i].command) >= COMMAND_STR_SIZE)
		{
#ifdef C_COMM_DEBUG
//			printf("\r\nERROR: Command \"%s\" had length out of range.\r\n", cmd_list[i].command);
			USART1_sendStr("\r\nERROR: Command \"");
			USART1_sendStr(cmd_list[i].command);
			USART1_sendStr("\" had length out of range.\r\n");
#endif
			return -1;
		}
		i++;
	}
	for (i = 0; i < REGISTER_SIZE; i++)
	{
		if (__register_list[i] == NULL)
		{
			__register_list[i] = cmd_list;
			__register_count++;
			break;
		}
	}
	if (i == REGISTER_SIZE)
	{
		// the register has fullfilled
		__register_list[__register_ring++] = cmd_list;
		if (__register_ring == REGISTER_SIZE)
		{
			__register_ring = 0;
		}
	}
	return 0;
}
int c_comm_process(char * line)
{
	if (c_comm_analyze(line) == 0)
	{
		int i = 0;
		int j = 0;
		for (i = 0; i < REGISTER_SIZE; i++)
		{
			if (__register_list[i] != NULL)
			{
				j = 0;
				while (__register_list[i][j].command != NULL)
				{
					if (memcmp(__register_list[i][j].command, __cmd, strlen(__cmd)) == 0)
					{
#ifdef C_COMM_DEBUG
//						printf("\r\nFound command \"%s\"\r\n", __cmd);
						USART1_sendStr("\r\nFound command \"");
						USART1_sendStr(__cmd);
						USART1_sendStr("\"\r\n");
#endif
						if (__register_list[i][j].action != NULL)
						{
#ifdef C_COMM_DEBUG
//							printf("\r\nExecute action\r\n");
							USART1_sendStr("\r\nExecute action\r\n");
#endif
							__register_list[i][j].action(__key, __val);
						}
						return 0;
					}
					j++;
				}
			}
		}

#ifdef C_COMM_DEBUG
//		printf("\r\nERROR: Command \"%s\" not regitered.\r\n", __cmd);
		USART1_sendStr("\r\nERROR: Command \"");
		USART1_sendStr(__cmd);
		USART1_sendStr("\" not regitered.\r\n");
#endif
		return -1;
	}
	else
	{
#ifdef C_COMM_DEBUG
//		printf("\r\nERROR: Communication process failed.\r\n");
		USART1_sendStr("\r\nERROR: Communication process failed.\r\n");
#endif
		return -1;
	}
	return 0;
}
