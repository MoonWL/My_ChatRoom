/*************************************************
* 服务器处理用户聊天操作实现文件
* 2016-5-5 wl实现
**************************************************/

#include "config.h"

int viewUserList(Message *msg , int sockfd);
int viewRecords(Message *msg , int sockfd);
int personalChat(Message *msg , int sockfd);
int groupChat(Message *msg,int sockfd);

int enterChat(int *fd)
{
	int sockfd,ret,n;
	
	/*声明消息变量*/
	Message message;
	User user;
	/*声明消息缓冲区*/
	char buf[MAX_LINE];
	
	//客户端的套接字地址
	struct sockaddr_in cliaddr;
	socklen_t len;
	sockfd = *fd;
	
	//获取客户的ip地址和端口号
	len = sizeof(cliaddr);
	if(getpeername(sockfd,(struct sockaddr *)&cliaddr,&len) < 0)
	{
		perror("getpeername error: ");
		printf("\n\n");
		exit(1);
	}

	while(1)
	{
		//接收用户发送的消息
		n = readn(sockfd , buf , sizeof(message));
		//printf("enterChat n = %d\n" , n);
		if(n == 0)
		{
			//关闭当前描述符
			printf("连接中断！\n");
			close(sockfd);
			return FAILED ;					
		}//if	
		else if(n < 0)
		{
			//关闭当前描述符
			//printf("客户enterChat界面突然退出\n");
			close(sockfd);
			return FAILED;					
		}//if			
				
		memcpy(&message , buf , sizeof(message));	
		message.sendAddr = cliaddr;
		//printf("server msgType = %d\n" , message.msgType);
		switch(message.msgType)
		{
			case VIEW_USER_LIST:
				{
					printf("来自用户\033[;31m%s\033[0m的查看在线用户列表请求！\n", message.sendName);
					/*转到查看在线用户列表处理函数*/
					ret = viewUserList(&message , sockfd);
					printf("查看在线列表：%s", stateMsg(ret));
					break;
				}
				
			case  GROUP_CHAT:
				{
					printf("\n来自\033[;31m%s\033[0m的群聊请求！\n", message.sendName);
					/*转到群聊处理函数*/
					ret = groupChat(&message , sockfd);
					printf("群聊：%s\n", stateMsg(ret));
					break;
				}
			
			case PERSONAL://检查聊天对象
				{
					User personal;
					strcpy(personal.userName , message.content);
					if(isOnLine(userList,&personal) == 1)
					{
						message.msgType = PERSONAL;
						message.msgRet = ALREADY_ONLINE;
						memcpy(buf , &message , sizeof(message));
						writen(sockfd , buf , sizeof(message));
					}
					else {
						message.msgType = PERSONAL;
						message.msgRet = ID_NOT_ONLINE;
						memcpy(buf , &message , sizeof(message));
						writen(sockfd , buf , sizeof(message));
					}
					break;
				}
				
			case PERSONAL_CHAT:
				{
					printf("\n来自\033[;31m%s\033[0m的私聊请求！\n", message.sendName);
					/*转到私聊处理函数*/
					ret = personalChat(&message , sockfd);
					printf("私聊：%s\n", stateMsg(ret));
				}
				break;		
				
			case EXIT:
				{
					printf("用户\033[;31m%s\033[0m退出登录！\n\n", message.sendName);
					memset(&user , 0 , sizeof(user));
					strcpy(user.userName , message.sendName);
					userList = deleteNode(userList , &user);
					memcpy(&message , buf , sizeof(message));
					message.msgType = EXIT;
					memcpy(buf , &message , sizeof(message));
					writen(sockfd , buf , sizeof(message));
					return ;
				}
			default:
				close(sockfd);		
				break;
		}//switch
		
	}//while
	return ;
}

int viewUserList(Message *msg , int sockfd)
{
	ListNode *p;
	int ret;

	/*消息发送缓冲区*/
	char buf[MAX_LINE];
	/*消息内容*/
	Message message;	
	memset(&message , 0 , sizeof(message));
	strcpy(message.sendName , (*msg).sendName);
	strcpy(message.recvName , (*msg).recvName);
	message.msgType = (*msg).msgType;
	
	/*查看在线用户*/
	p = userList;
	if(p == NULL)
	{
		/*改变消息类型为RESULT*/
		message.msgType = RESULT;
		strcpy(message.content, stateMsg(ALL_NOT_ONLINE));
		memset(buf , 0 , MAX_LINE);
		memcpy(buf , &message , sizeof(message));
		writen(sockfd , buf , sizeof(message));
		return ALL_NOT_ONLINE;
	}//if
	else{
		/*否则消息类型不变*/
		strcpy(message.content , "");
		while(p!=NULL)
		{
			strcat(message.content , "\t");
			strcat(message.content , (p->user).userName);
			p = p->next;
		}//while
		memset(buf , 0 , MAX_LINE);
		memcpy(buf , &message , sizeof(message));
		writen(sockfd , buf , sizeof(message));
		printf("查看在线列表结果：\033[;31m%s\033[0m\n", message.content);
	}
	return SUCCESS;
}

int groupChat(Message *msg , int sockfd)
{
	ListNode *p;
	
	int ret;

	/*声明数据库变量*/
	sqlite3 *db;
	sqlite3_stmt *stmt;
	const char *tail;
	/*声明sql语句存储变量*/
	char sql[256];
	
	/*消息发送缓冲区*/
	char buf[MAX_LINE];
	/*消息内容*/
	Message message;	
	memset(&message , 0 , sizeof(message));
	strcpy(message.sendName , (*msg).sendName);
	strcpy(message.recvName , (*msg).recvName);
	strcpy(message.content,(*msg).content);
	strcpy(message.msgTime , (*msg).msgTime);
	message.msgType = (*msg).msgType;
	//printf("message.msgType:%d\n",message.msgType);
	
	/*查看在线用户*/
	p = userList;
	/*除了自己无人在线*/
	if(p == NULL)
	{
		/*改变消息类型为RESULT*/
		message.msgType = RESULT;
		message.msgRet = ALL_NOT_ONLINE;
		strcpy(message.content, stateMsg(ALL_NOT_ONLINE));	
		memset(buf , 0 , MAX_LINE);
		memcpy(buf , &message , sizeof(message));
		writen(sockfd , buf , sizeof(message));
		return ALL_NOT_ONLINE;
	}//if
	//向所有在线用户发送消息
	
	while(p!=NULL)
	{
			//if(strcmp((p->user).userName , message.sendName) != 0)
			//{
				memset(buf , 0 , MAX_LINE);
				memcpy(buf , &message , sizeof(message));
				//printf("发送成功！\n");
				writen((p->user).sockfd , buf , sizeof(message));
				printf("发送给%s成功！\n",(p->user).userName);
			//}//else
			p = p->next;
	}//while
	return SUCCESS;
}

int personalChat(Message *msg , int sockfd)
{
	ListNode *p;
	
	int ret;

	/*声明数据库变量*/
	sqlite3 *db;
	sqlite3_stmt *stmt;
	const char *tail;
	/*声明sql语句存储变量*/
	char sql[256];

	/*消息发送缓冲区*/
	char buf[MAX_LINE];
	/*消息内容*/
	Message message;	
	memset(&message , 0 , sizeof(message));	
	memcpy(&message,msg,sizeof(message));
	/*消息发送对象和接收对象相同*/
	if(strcmp((*msg).sendName , (*msg).recvName) == 0)
	{
		printf("消息不能发送到自己！\n");
		/*改变消息类型为RESULT*/
		message.msgType = RESULT;
		strcpy(message.content, stateMsg(MESSAGE_SELF));
		memset(buf , 0 , MAX_LINE);
		memcpy(buf , &message , sizeof(message));
		writen(sockfd , buf , sizeof(message));
		return MESSAGE_SELF;
	}//if

	/*查找接收信息用户*/
	p = userList;
	while(p != NULL && strcmp((p->user).userName , (*msg).recvName) != 0)
	{		
		p = p->next;
	}//while

	if(p == NULL)
	{
		printf("该用户不在线！\n");
		/*改变消息类型为RESULT*/
		message.msgType = RESULT;
		strcpy(message.content, stateMsg(ID_NOT_ONLINE));
		memset(buf , 0 , MAX_LINE);
		memcpy(buf , &message , sizeof(message));
		writen(sockfd , buf , sizeof(message));
		return ID_NOT_ONLINE;
	}//if
	else{
		memset(buf , 0 , MAX_LINE);
		memcpy(buf , &message , sizeof(message));
		writen((p->user).sockfd , buf , sizeof(message));
		printf("发送给%s成功！\n",(p->user).userName);
		return SUCCESS;
	}//else	
}

