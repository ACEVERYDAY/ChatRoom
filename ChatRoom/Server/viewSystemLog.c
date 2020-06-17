#include <stdio.h>
#include <string.h>
#include "server.h"

int viewSystemLog() 
{
	FILE *fp;
	char line[MAX_BUFF_SIZE];
	if(!(fp = fopen("../SystemLog/daemon.log", "a+"))) {
		perror("file open error!");
		return FILE_OPEN_ERROR;
	}
	while(fgets(line, MAX_BUFF_SIZE, fp)) {
		printf("%s", line);
	}
	fclose(fp);	
	return READ_SUCCESS;
}
