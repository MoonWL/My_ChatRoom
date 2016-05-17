/***********************************************************
 * 客户端程序代码client.c
 * 2016-5-4 wl实现
***********************************************************/

#include"client.h"

/******************************
 * 函数名：main
 * 功能：聊天室客户端main函数入口
 * 参数：参数个数argc,要连接的服务器地址argv
 * 返回值: 正常退出返回0，否则返回1
************************************/

int main(int argc,char *argv[])
{
	int sockfd,ret,n;
	char choice[20];
	struct sockaddr_in servaddr;
	
	//声明消息变量
	Message message;
	//声明消息缓冲区
	char buf[MAX_LINE];
	
	//用户信息
	User user;
	strcpy(user.userName,"***");
	user.speak = 1;
	
	//判断是否为合法输入
	if(argc != 2)
	{
			perror("usage: tcpcli <IPaddress>");
			exit(1);
	}
	//(1)创建套接字
	if((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		perror("socket error");
		exit(1);
	}
	
	//(2)设置服务器的套接字地址
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	if(inet_pton(AF_INET,argv[1],&servaddr.sin_addr) < 0)
	{
		printf("inet_pton error for %s\n",argv[1]);
		exit(1);
	}
	
	//(3)发送服务器连接请求
	if(connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0)
	{
		perror("connect error");
		exit(1);
	}
	system("clear");
	while(1){
		
		//(4)显示聊天室主界面
		mainInterface();
		//setbuf(stdin,NULL);//设置标准输入不带缓冲
		fflush(stdin);
		scanf("%s",choice);
		fflush(stdin);
		//setbuf(stdin,NULL);//设置标准输入不带缓冲
		ret = strlen(choice);
		while(ret != 1 || (choice[0] != '1' && choice[0] != '2' && choice[0] != '3' && choice[0] != '4'))
		{
			printf("未找到命令,请重新输入!\n");
			//setbuf(stdin,NULL);//设置标准输入不带缓冲
			fflush(stdin);
			scanf("%s",choice);
			fflush(stdin);
			ret = strlen(choice);
			//setbuf(stdin,NULL);//设置标准输入不带缓冲
		}
		
		switch(choice[0]-'0')
		{
			case REGISTER://发送注册请求
				memset(&message,0,sizeof(message));
				memset(buf,0,MAX_LINE);
				message.msgType = REGISTER;
				strcpy(message.content,"");
				//发送者的套接字地址由服务器根据getpeername函数获取
				message.recvAddr = servaddr;//接收者的套接字地址
				memcpy(buf,&message,sizeof(message));
				//printf("消息类型%d\n",message.msgType);
				if((n = writen(sockfd,buf,sizeof(message))) <= 0){
					perror("send error: ");
					printf("\n\n");
					exit(1);
				}
				else if(n == sizeof(message)){
					//printf("已经发送了%d字节数据\n",n);
					//printf("消息类型%d\n",message.msgType);
				}
				ret = registerUser(sockfd);
				printf("%s\n",stateMsg(ret));
				break;
				
			case LOGIN://发送登录请求
				memset(&message,0,sizeof(message));
				memset(buf,0,MAX_LINE);
				message.msgType = LOGIN;
				strcpy(message.content,"");
				message.recvAddr = servaddr;//接收者的套接字地址
				memcpy(buf,&message,sizeof(message));
				//printf("消息类型%d\n",message.msgType);
				if(writen(sockfd,buf,sizeof(message)) < 0){
					perror("send error: ");
					printf("\n\n");
					exit(1);
				}
				ret = loginUser(sockfd);
				break;
				
			case HELP:
				helpInterface();
				break;
				
			case EXIT:
				close(sockfd);
				printf("退出聊天室！\n");
				exit(0);
				break;
				
			default:
				printf("unknow operation.\n");
				close(sockfd);
				return 1;
				break;
		}
	}
	close(sockfd);
	return 0;	
}

