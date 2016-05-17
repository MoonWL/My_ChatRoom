/***************************************************
 * 客户端用户注册文件
 * 2016-5-4 wl实现
*****************************************************/

#include"client.h"

/******************************************************
 * 函数名：registeruser
 * 功能：用户注册函数实现
 * 参数：与客户端通信的套接字描述符sockfd
 * 返回值：成功注册返回Success,否则返回异常类型
*********************************************************/

int registerUser(int sockfd)
{
	int n;
	
	//声明用户注册信息
	User user;
	char buf[MAX_LINE];
	Message message;
	
	memset(buf,0,MAX_LINE);
	readn(sockfd,buf,sizeof(message));
	//printf("接受注册允许\n");
	
	/*获取用户输入*/
	printf("请输入注册用户名:\n");
	memset(user.userName , 0 , sizeof(user.userName));
	fflush(stdin);
	scanf("%s" , user.userName);
	fflush(stdin);
	printf("user.UserName = %s\n" , user.userName);
	
	printf("请输入注册用户密码:\n");
	memset(user.password , 0 , sizeof(user.password));
	fflush(stdin);
	scanf("%s" , user.password);
	fflush(stdin);
	printf("user.password = %s\n" , user.password);
	
	//当前的端口号
	user.sockfd = sockfd;
	//当前用户允许发言
	user.speak = YES;
	//地址由服务器根据getpeername获取
	
	memset(buf,0,MAX_LINE);
	memcpy(buf,&user,sizeof(user));
	//sleep(1);
	writen(sockfd,buf,sizeof(user));//发送用户注册信息
	//printf("发送注册信息\n");
	
	//接受注册结果
	memset(buf,0,MAX_LINE);
	if((n = readn(sockfd,buf,sizeof(message))) <=0){
		printf("服务器已断开连接！\n");
		exit(0);
	}
	//printf("接收字节数：%d\n",n);
	memset(&message,0,sizeof(message));
	memcpy(&message,buf,sizeof(message));
	//printf("aaa%s\n",stateMsg(message.msgRet));
	return message.msgRet;
} 

