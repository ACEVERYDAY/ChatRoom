#include <stdio.h>
#include <string.h>
#include "server.h"

// 验证登录 , flag＝0代表登录，flag=1代表check user exist
int varifyLogin(char *check_name, char *check_pwd, int flag) 
{
	// 打开文件
	FILE *fp;
	char usr_name[MAX_BUFF_SIZE], usr_pwd[MAX_BUFF_SIZE];
	if(!(fp = fopen("../LoginInfo/LogInfo.txt", "r"))) {
		perror("file open error!");
		return FILE_OPEN_ERROR;
	} 
	// 逐行读取文件 
	while(fscanf(fp, "%s %s", usr_name, usr_pwd) != EOF) {
		if(!strcmp(check_name, usr_name)) {
			if(flag)
				return USR_HAS_EXIST;
			if(!strcmp(check_pwd, usr_pwd))
				return VARIFY_SUCCESS;
		}
	}
	fclose(fp);
	return 0;
}
