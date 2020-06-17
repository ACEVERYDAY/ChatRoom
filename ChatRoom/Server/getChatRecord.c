#include <stdio.h>
#include <string.h>
#include "server.h"

char *getChatRecord() 
{
	FILE *fp;
	char line[MAX_BUFF_SIZE];
	static char chat_record[MAX_CHAT_RSIZE];
	// memset(chat_record, 0, sizeof(chat_record));
	if(!(fp = fopen("../ChatRecord/ChatInfo.txt", "r+"))) {
		perror("file open error!");
		// return FILE_OPEN_ERROR;
	}
	while(fgets(line, MAX_BUFF_SIZE, fp)) {
		// printf("%s", line);
		strcat(chat_record, line);
		strcat(chat_record, "\n");
	}
	// printf("%s", chat_record);
	fclose(fp);	
	//return READ_SUCCESS;
	return chat_record;
}
