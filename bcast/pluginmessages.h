#ifndef PLUGINMESSAGES_H
#define PLUGINMESSAGES_H


#include "messages.h"


class PluginMessages
{
public:
	PluginMessages(int input_flag, int output_flag, int message_id = -1);
	~PluginMessages();
	
	void send_message(char *text);
	void recieve_message(char *text);
	
	void send_message(int command, char *text);      
	void send_message(long command, long value);      
	void send_message(long command, long value1, long value2);      
	void send_message(int command);      

	void recieve_message();     // returns the command
	void recieve_message(int *command, char *text);
	void recieve_message(int *command, long *value);
	void recieve_message(long *value1, long *value2);
	void recieve_message(int *command, long *value1, long *value2);
	
	Messages *messages;
	int input_flag, output_flag;
};








#endif
