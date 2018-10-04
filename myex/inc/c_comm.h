#ifndef _c_comm_h_
#define _c_comm_h_

#ifdef __cplusplus
extern "C" {
#endif

#define COMMAND_STR_SIZE 10
#define KEY_STR_SIZE 10
#define VALUE_STR_SIZE 32

#define REGISTER_SIZE 10

	struct c_comm
	{
		const char * command;
		int(*action)(const char * key, const char * value);
	};

	int c_comm_analyze(char * line);

	int c_comm_register(struct c_comm cmd_list[]);
	int c_comm_process(char * line);

#ifdef __cplusplus
}
#endif

#endif
