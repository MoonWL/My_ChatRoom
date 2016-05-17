/****************************************************
 * 客户端界面设计
 * 2016-5-4 wl设计
*******************************************************/

#include"config.h"

/*******************************************************
 * 函数名：mainInterface
 * 功能：登录界面
 * 传入参数：无
 * 返回值：无
**********************************************************/

int mainInterface()
{
	printf("\033[;31m               --------------------------------------------\033[0m\n");
	printf("\033[;31m                        欢迎进入亮仔聊天室2.0版            \033[0m\n");
	printf("\033[;31m                                1.注册                     \033[0m\n");
	printf("\033[;31m                                2.登录                     \033[0m\n");
	printf("\033[;31m                                3.帮助                     \033[0m\n");
	printf("\033[;31m                                4.退出                     \033[0m\n");
	printf("\033[;31m               --------------------------------------------\033[0m\n");
}

int helpInterface()
{
	char choice[20];
	int ret;
	system("clear");
	printf("\033[;32m               --------------------------------------------\033[0m\n");
	printf("\033[;32m                              欢迎进入帮助界面             \033[0m\n\n");
	printf("\033[;32m                        提示：第一次使用请先注册           \033[0m\n");
	printf("\033[;32m                              注册之后可使用帐号和密码登录 \033[0m\n");
	printf("\033[;32m                              请按q退出帮助界面            \033[0m\n");
	printf("\033[;32m               --------------------------------------------\033[0m\n");
	fflush(stdin);
	scanf("%s",choice);
	//setbuf(stdin,NULL);//设置标准输入不带缓冲
	ret = strlen(choice);
	while(ret != 1 || choice[0] != 'q')
	{
		printf("未找到命令,请重新输入!\n");
		//setbuf(stdin,NULL);//设置标准输入不带缓冲
		fflush(stdin);
		scanf("%s",choice);
		ret = strlen(choice);
		//setbuf(stdin,NULL);//设置标准输入不带缓冲
	}
	system("clear");
}


void chatInterface(char userName[])
{
	    printf("\033[;31m                                你好，%s                 \033[0m\n", userName);
		printf("\033[;32m          ---------------------------------------------\033[0m\n");
		printf("\033[;32m	                       1. 查看在线用户               \033[0m\n");
		printf("\033[;32m	                       2. 进入群聊模式               \033[0m\n");
		printf("\033[;32m	                       3. 进入私聊模式               \033[0m\n");
		printf("\033[;32m	                       4. 退出登录                   \033[0m\n");
		printf("\033[;32m          ---------------------------------------------\033[0m\n\n\n");
}

void groupInterface(char userName[],int sockfd)
{
	char buf[MAX_LINE];
	Message message;
	
	system("clear");
	printf("\033[;31m你好 %s !您已进入群聊模式，输入\033[0m \033[;32mq!\033[0m \033[;31m退出群聊模式！\033[0m\n", userName);
	//显示当前在线用户
	message.msgType = VIEW_USER_LIST;
	strcpy(message.sendName,userName);
	memcpy(buf , &message , sizeof(message));
	writen(sockfd , buf , sizeof(message));	
}

void personalInterface(char userName[],int sockfd)
{
	char buf[MAX_LINE];
	Message message;
	
	system("clear");
	printf("\033[;31m你好 %s !您已进入私聊模式，输入\033[0m \033[;32mq!\033[0m \033[;31m退出群聊模式！\033[0m\n", userName);
	//显示当前在线用户
	message.msgType = VIEW_USER_LIST;
	strcpy(message.sendName,userName);
	memcpy(buf , &message , sizeof(message));
	writen(sockfd , buf , sizeof(message));	
}
