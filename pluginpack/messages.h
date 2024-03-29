#ifndef MESSAGES_H
#define MESSAGES_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MESSAGE_TO_PLUGIN 1
#define MESSAGE_FROM_PLUGIN 2
#include "messages.inc"

class Messages
{
public:
	Messages(int input_flag, int output_flag, int id = -1);
	~Messages();

	char* get_message_buffer();

	int read_message(char *text);
	char* read_message_raw();        // return the raw text in the buffer
	int read_message(int *command, char *text);
	long read_message();    // return the number contained in the message
	int read_message(long *value1, long *value2);
	int read_message(long *command, long *value1, long *value2);
	int read_message(long *command, long *value1, long *value2, long *value3);
	float read_message_f();
	int read_message_f(float *value1, float *value2);
	int read_message_f(float *value1, float *value2, float *value3);
	int read_message_f(float *value1, float *value2, float *value3, float *value4);

	int write_message(const char *text);
	int write_message_raw();                 // send the text currently in the buffer
	int write_message(int command, char *text);
	int write_message_flagged(int output_flag, int command);
	int write_message(int number);
	int write_message(long command, long value);
	int write_message(long command, long value1, long value2);
	int write_message(long command, long value1, long value2, long value3);
	int write_message_f(float number);
	int write_message_f(float value1, float value2);
	int write_message_f(float value1, float value2, float value3);
	int write_message_f(float value1, float value2, float value3, float value4);
	int get_id();

	int msgid;
	int client;
	int input_flag;
	int output_flag;
	struct buffer_ {
		long mtype;
		char text[MESSAGESIZE];
	} buffer;
};

#endif
