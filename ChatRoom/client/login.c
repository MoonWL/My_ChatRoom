/********************************************************
 * 客户端用户登录实现文件
 * 2016-5-5 wl实现
***********************************************************/

#include"config.h"

/*******************************************************
 * 函数名：loginUser
 * 功能：用户登录函数实现
 * 参数：用于通信的套接字描述符
 * 返回值：成功登录返回SUCCESS，否则返回异常类型
********************************************************/

int loginUser(int sockfd)
{
	int ret;
	//声明用户登录信息
	User user;
	char buf[MAX_LINE];
	Message message;
	
	//获取用户输入
	printf("请输入用户名：\n");
	memset(user.userName , 0 , sizeof(user.userName));
	scanf("%s" , user.userName);
	printf("user.UserName = %s\n" , user.userName);
	
	printf("请输入用户密码：\n");
	memset(user.password , 0 , sizeof(user.password));
	scanf("%s" , user.password);
	printf("user.password = %s\n" , user.password);
	
	memset(buf,0,MAX_LINE);
	if(readn(sockfd,buf,sizeof(message)) != sizeof(message))
		return FAILED;
	//发送用户登录信息到服务器
	memset(buf,0,MAX_LINE);
	memcpy(buf,&user,sizeof(user));
	writen(sockfd,buf,sizeof(user));
	
	memset(buf,0,MAX_LINE);
	if(readn(sockfd,buf,sizeof(message)) <= 0)
	{
		printf("服务器已断开连接！\n");
		exit(0);
	}
	memset(&message,0,sizeof(message));
	memcpy(&message,buf,sizeof(message));
	
	printf("%s\n",message.content);
	if(message.msgRet == SUCCESS)
	{
		//进入聊天模式
		enterChat(&user,sockfd);
	}
	return message.msgRet;
}

