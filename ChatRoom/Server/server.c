#include <time.h>
#include <stdio.h>
#include "server.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>


// 服务器的作用实际上是对聊天内容进行处理以及转发
int main(int argc, char *argv[]) 
{

	userInterface();

	return 0;
}


// 注册用户
int registerUser(char *usr_name, char *usr_pwd) 
{
	FILE *fp;
	if(!(fp = fopen("../LoginInfo/LogInfo.txt", "a+"))) {
		perror("file open error!");
		return FILE_OPEN_ERROR;
	}
	fprintf(fp, "%s %s\n", usr_name, usr_pwd);
	fclose(fp);
	return WRITE_SUCCESS;
}

// 验证是否为守护进程
int isDaemonProcess(char *str1, char *str2) 
{
	// 随便找了一个MD5码作为密码校验(#分割，分别对比验证)
	// 5b08410da6d1703#4b8795d7bc45f58ec
	char *tmp1 = "5b08410da6d1703", *tmp2 = "4b8795d7bc45f58ec";
	if(!strcmp(str1, tmp1) && !strcmp(str2, tmp2)) {
		return 1;
	}
	return 0;
}

// 获取系统时间
char *getLocalTime() 
{
	time_t timep;
	time(&timep);
	//return asctime(gmtime(&timep));
	return ctime(&timep);
}

// zhengxing zhuan zifuxing
char *itoa(int length) 
{
	static char str[6];
	sprintf(str, "%d", length);
	return str;
}

// 初始化套接字信息
int initSocket() 
{
     
	// 建立套接字，获得套接字文件描述符 
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == -1){
		perror("socket");
		return 0;
	}

	max_sock = sock;

	bzero(&server, sizeof(server));
	server.sin_port = htons(PORT);
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_family = AF_INET;
	if(bind(sock, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1) {
		perror("bind");
		return 0;
	}

	// listen(sock, MAX_CLIENT);
	if(listen(sock, MAX_CLIENT) == -1) {
		perror("listen");
		return 0;
	}
	return 1;
}

// 监听所有MAX_CLIENT个客户端的信息
void listen2Clients() 
{
	
	FILE *fp;
	FILE *tp;
	int i = 0, j = 0;
	
	if(!(fp = fopen("../ChatRecord/ChatInfo.txt", "a+"))) {
		perror("file open error!\n");
		exit(0);
	}
	fprintf(fp, "\n_____________________________________\n");
	
	// 定义文件描述符集
	fd_set rfd;
	while(1) {

		FD_ZERO(&rfd);
		FD_SET(sock, &rfd);
		FD_SET(STDIN_FILENO, &rfd);
		// int t = read(STDIN_FILENO, rbuf,sizeof(rbuf));
		for(i = 0;i < MAX_CLIENT; i++) {
			if(cli[i].c_fd == 0)
				continue;
			// 在文件描述符集rfd中增加当前的新的文件描述符 
			FD_SET(cli[i].c_fd, &rfd);
		}
		// 检索文件描述符集所有客户端哪个可以读 
		switch(select(max_sock+1, &rfd, NULL, NULL, NULL)) {
			case 0:
				continue;
			case -1:
				continue;
			default:
				if(FD_ISSET(STDIN_FILENO, &rfd)) {
					bzero(rbuf, sizeof(rbuf));
					// 读取标准输入，并送往服务器
					int rsize = read(STDIN_FILENO, rbuf, sizeof(rbuf));
					if(rsize > 0) {
						// success
						// 退出，将用户退出状态、服务器退出状态写入daemon.log，关闭守护进程、服务器连接
						if(!strcmp(rbuf, "2\n")) {
							// daemon_fd
							for(i = 0;i < MAX_CLIENT;i++) {
								if(cli[i].online)
								{
									sprintf(rbuf, "%s quit at %s", cli[i].name, getLocalTime());
									sprintf(temp_msg, "%s#%s", itoa(strlen(rbuf)), rbuf);
									send(daemon_fd, temp_msg, strlen(temp_msg), 0);
									close(cli[i].c_fd);							
								}
							}
							sprintf(rbuf, "system close at %s", getLocalTime());
							sprintf(temp_msg, "%s#%s", itoa(strlen(rbuf)), rbuf);
							printf("%s\n", rbuf);
							fflush(stdout);
							send(daemon_fd, temp_msg, strlen(temp_msg), 0);
							close(daemon_fd);
							sleep(2);
							system("killall daemon");
							return;
						}
					}
				}
				// 若已经将socket描述符加入到可读文件的描述符集中
				if(FD_ISSET(sock, &rfd)) {
					// 遍历所有客户端
					for(i = 0;i < MAX_CLIENT; i++) {
						// 首次检测出客户端开启
						if(cli[i].c_fd == 0) {
							len = sizeof(struct sockaddr);
							// 服务器接收客户的连接请求成功连接后记录客户端的文件描述符 
							cli[i].c_fd = accept(sock, (struct sockaddr *)&client, &len);
							// 连接出错 
							if(cli[i].c_fd == -1) {
								perror("accept");
								cli[i].c_fd = 0;
								break;
							}
							// 客户端仅仅连接到服务器时，用户未得到验证，因此用户在线状态为 0 
							cli[i].online = 0;
							if(cli[i].c_fd > max_sock)
								max_sock = cli[i].c_fd;
							printf("accept!\n");
							break;
						}
					}
				}
		
			for(i = 0; i < MAX_CLIENT; i++) {
				// 跳过未开启的客户端 
				if(cli[i].c_fd == 0)
					continue;
				if(FD_ISSET(cli[i].c_fd, &rfd)) {
					int rsize, psize;
					bzero(rbuf, sizeof(rbuf));
					// 接收来自第 i 个客户端的账号，并存放到rbuf中 
					rsize = recv(cli[i].c_fd, rbuf, sizeof(rbuf), 0);
					// printf("%s\n", rbuf);
					
					char m[] = "#", *usr_info[3];
					char *p = strtok(rbuf, m);
					usr_info[0] = p;
					usr_info[1] = strtok(NULL, m);
					usr_info[2] = strtok(NULL, m);
					// printf("%s %s %s\n", usr_info[0], usr_info[1], usr_info[2]);
					
					if(rsize > 0) {
						// 如果客户 i 在线，服务器转发消息，并将消息打印到本服务器
						if(cli[i].online) {
							for(j = 0;j < MAX_CLIENT; j++) {
								// 不将消息发给自己或者不在线的用户 (将消息发给他人)
								if(cli[j].c_fd==0 || j==i || cli[j].online==0)
									continue;
								
								bzero(wbuf, sizeof(wbuf));
								sprintf(wbuf, "%s%s: %s", getLocalTime(), cli[i].name, rbuf);
								send(cli[j].c_fd, wbuf, strlen(wbuf), 0);
							}
							printf("%s%s: %s\n", getLocalTime(), cli[i].name, rbuf);
							fprintf(fp, "%s%s: %s\n", getLocalTime(), cli[i].name, rbuf);
							// 这里做个记录，不及时刷新缓冲区的话，当服务端以Ctrl+C终止时，不能成功写入文件
							fflush(fp);				
							if(!strcmp(rbuf, "#bye\n")) {
								send(cli[i].c_fd, "quit", 4, 0);
								sprintf(login_msg, "%s quit at %s", cli[i].name, getLocalTime());
								sprintf(temp_msg, "%s#%s", itoa(strlen(login_msg)), login_msg);
								send(daemon_fd, temp_msg, strlen(temp_msg), 0);
								close(cli[i].c_fd);
								cli[i].c_fd = 0;
								cli[i].online = 0;
								memset(cli[i].name, 0, sizeof(cli[i].name));
							}
							else if(!strcmp(rbuf, "#chat record\n")) {
								char *tmpr = getChatRecord();
								send(cli[i].c_fd, tmpr, strlen(tmpr), 0);
							}
						}
						else {
							// 若刚刚启动客户端，添加用户进入列表并提示成功登录
							// 传给守护进程的数据格式：数据报长度#数据（解决 tcp 粘包问题）***重点记录***
							if(isDaemonProcess(usr_info[0], usr_info[1])) {
								char tmp_ch[MAX_BUFF_SIZE];
								sprintf(tmp_ch, "\nsystem start at %s", getLocalTime());
								sprintf(temp_msg, "%s#%s", itoa(strlen(tmp_ch)), tmp_ch);
								daemon_fd = cli[i].c_fd;
								send(daemon_fd, temp_msg, strlen(temp_msg), 0);
							}
							// option 2: login							
							else if(!strcmp(usr_info[2], "2") && varifyLogin(usr_info[0],usr_info[1],0)==VARIFY_SUCCESS) {
								strcpy(login_msg, "login successful!");
								strcpy(cli[i].name, rbuf);
								cli[i].online = 1;
								send(cli[i].c_fd, login_msg, strlen(login_msg), 0);
								sprintf(login_msg, "%s log-in at %s", cli[i].name, getLocalTime());
								sprintf(temp_msg, "%s#%s", itoa(strlen(login_msg)), login_msg);
								send(daemon_fd, temp_msg, strlen(temp_msg), 0);
							}
							// option 1: register
							else if(!strcmp(usr_info[2], "1") && varifyLogin(usr_info[0],usr_info[1],1)==USR_HAS_EXIST) {
								strcpy(login_msg, "user has exist!");
								send(cli[i].c_fd, login_msg, strlen(login_msg), 0);
							}
							else if(!strcmp(usr_info[2], "1") && registerUser(usr_info[0],usr_info[1])==WRITE_SUCCESS) {
								strcpy(login_msg, "register successful!");
								send(cli[i].c_fd, login_msg, strlen(login_msg), 0);
							}
							else {
								// 验证失败 
								send(cli[i].c_fd, "quit", 4, 0);
								close(cli[i].c_fd);
								cli[i].c_fd = 0;
								cli[i].online = 0;
								memset(cli[i].name, 0, sizeof(cli[i].name));
							}

							
						}
						
					}
				}
			}

		}
	
	}
	fclose(fp);
}

// 用户接口界面
void userInterface() 
{
	int option = 0;

	printf("----------------------------------\n");
	printf("----------SERVER MENU-------------\n");
	printf("--------1. START SYSTEM-----------\n");
	printf("--------2. CLOSE SYSTEM-----------\n");
	printf("--------3. VIEW SYSTEM LOG--------\n");
	printf("----------------------------------\n");
	
	scanf("%d", &option);
	switch(option) {
		case 1:
			if(!initSocket())
				exit(0);
			// 拼接系统开始时间的字符串
			
			// 以daemon.c进程的文件描述符为基础向此守护进程发送拼接的字符串，并由守护进程写入到log文件
			system("../Daemon/daemon");
			listen2Clients();
			break;
		case 2:
			break;
		case 3:
			viewSystemLog();
			userInterface();
			break;
	}
}
