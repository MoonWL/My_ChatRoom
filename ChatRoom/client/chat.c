/**************************************************************
 * 客户端用户聊天实现文件
 * 2016-5-5 wl实现
***************************************************************/

#include"config.h"

void recvMsg(int *sockfd);
int personalRet;
/************************************************************
 * 函数名：enterChat
 * 功能：用户登录成功后进入聊天模式
 * 参数：user--当前用户，sockfd套接字描述符
 * 返回值：正常退出返回0，否则返回1
*************************************************************/

void enterChat(User *user,int sockfd)
{
	char choice[20];
	int ret,len;
	char c,buf[MAX_LINE],str[MAX_LINE];
	Message personal;
	
	Message message;
	time_t timep;
	
	//创建一个线程去接收消息，本线程输入消息
	pthread_t pid;
	ret = pthread_create(&pid , NULL , (void *)recvMsg , (void *)&sockfd);
	if(ret != 0)
	{
		printf("软件异常，请重新登录！\n");
		close(sockfd);
		exit(1);
	}
	
	chatInterface((*user).userName);
	while(1)
	{
		fflush(stdin);
		scanf("%s",choice);
		//setbuf(stdin,NULL);//设置标准输入不带缓冲
		ret = strlen(choice);
		while(ret != 1 || (choice[0] != '1' && choice[0] != '2' && choice[0] != '3' 
			&& choice[0] != '4'))
		{
			printf("未找到命令,请重新输入!\n");
			//setbuf(stdin,NULL);//设置标准输入不带缓冲
			fflush(stdin);
			scanf("%s",choice);
			ret = strlen(choice);
			//setbuf(stdin,NULL);//设置标准输入不带缓冲
		}
		
		switch(choice[0]-'0')
		{
			case 1: /*查看当前在线用户列表*/
				message.msgType = VIEW_USER_LIST;
				strcpy(message.sendName,(*user).userName);
				memcpy(buf , &message , sizeof(message));
				writen(sockfd , buf , sizeof(message));						
				break;
			
			case 2://进入群聊模式
				groupInterface((*user).userName,sockfd);
				getchar();
				sleep(1);
				printf("正在建立聊天连接...\n");
				printf("\n请输入内容：\n");
		group:	message.msgType = GROUP_CHAT;
				fflush(stdin);
				fgets(message.content , MAX_LINE , stdin);
				//gets_s(message.content,sizeof(message.content));
				fflush(stdin);
				//printf("聊天内容：%s\n",message.content);
				if(strcmp(message.content,"q!\n") == 0){
					//退出群聊模式
					printf("已退出群聊模式！\n");
					chatInterface((*user).userName);
					break;
				}
				//printf("\n");
				strcpy(message.sendName,(*user).userName);
				strcpy(message.recvName , "");
				
				/*获得当前时间*/
				time(&timep);
				strcpy(message.msgTime , ctime(&timep));
				memcpy(buf , &message , sizeof(message));
				writen(sockfd , buf , sizeof(message));
				printf("\t\t\t\t\t\t\033[;32m发送消息成功!\033[0m\n");
				goto group;
			
			case 3: //进入私聊模式
				personalInterface((*user).userName,sockfd);
				sleep(1);
				getchar();
				while(1){
					printf("\n请输入聊天对象：\n");
					fflush(stdin);
					fgets(personal.content , MAX_LINE , stdin);
					//gets_s(personal.content,sizeof(personal.content));
					//scanf("%s" , personal.username);
					fflush(stdin);
					if(strcmp(personal.content,"q!\n") == 0){
						//退出私聊模式
						printf("已退出私聊模式！\n");
						goto quit;
					}
					//由于fgets会读入\n,而User中的userName是不带\n的，故要去掉\n
					len = strlen(personal.content);
					personal.content[len-1] = 0;
					personal.msgType = PERSONAL;
					memset(buf,0,MAX_LINE);
				    memcpy(buf,&personal,sizeof(personal));
					writen(sockfd,buf,sizeof(personal));//发送聊天对象
					printf("正在建立聊天连接...\n");
					//与消息线程冲突
					sleep(1);
					//printf("接受聊天对象成功！\n");
					if(personalRet == ALREADY_ONLINE )
					{
						if(strcmp(personal.content,(*user).userName) == 0)
							printf("\033[;31m聊天对象不能为自己！\033[0m\n\n");
						else  {
							printf("正在与 \033[;31m%s\033[0m 聊天！\n\n",personal.content);
							break;
						}							
					}
					else printf("\033[;31m聊天对象不正确！\033[0m\n\n");
				}
				strcpy(message.recvName , personal.content);

				printf("请输入聊天内容：\n");
		personal:	message.msgType = PERSONAL_CHAT;
				fflush(stdin);			
				fgets(message.content , MAX_LINE , stdin);
				fflush(stdin);
				//(message.content)[strlen(message.content) - 1] = '\0';

				if(strcmp(message.content,"q!\n") == 0){
					//退出私聊模式
					printf("已退出私聊模式！\n");
		quit:		chatInterface((*user).userName);
					break;
				}
				//printf("\n");
				strcpy(message.sendName,(*user).userName);
				strcpy(message.recvName , personal.content);
				/*获得当前时间*/
				time(&timep);
				strcpy(message.msgTime , ctime(&timep));
				memcpy(buf , &message , sizeof(message));
				writen(sockfd , buf , sizeof(message));
				printf("\t\t\t\t\t\t\033[;32m发送消息成功!\033[0m\n");
				goto personal;
		
			 case 4:
				message.msgType = EXIT;
				strcpy(message.sendName,(*user).userName);
				memcpy(buf , &message , sizeof(message));
				writen(sockfd , buf , sizeof(message));
				pthread_join(pid,NULL);
				return ;
			default:
				//close(sockfd);
				//exit(1);
				break;
		}//switch
		
		
	}	
	
}



void recvMsg(int *sockfd)
{
	int nRead;
	int connfd = *sockfd;
	char buf[MAX_LINE] , str[MAX_LINE];
	Message message;
	
	time_t timep;

	while(1)
	{
		/*接收服务器发来的消息*/
		nRead = readn(connfd , buf , sizeof(message));
		/*recv函数返回值 <0 出错  =0 链接关闭  >0接收到的字节数*/
		if(nRead < 0){
			printf("recv error!\n");
			continue;
		}
		if(nRead == 0)
		{
			printf("已断开连接，请重新登录！\n");
			close(connfd);
			exit(1);
		}//if

		memset(&message , 0 , sizeof(message));
		memcpy(&message , buf , sizeof(message));

		//printf("message.msgType:%d\n",message.msgType);
		switch(message.msgType)
		{
			case VIEW_USER_LIST:
				printf("当前在线用户有：\n\033[;31m%s\033[0m\n", message.content);
				break;
				
			case GROUP_CHAT:
				sprintf(str , "\033[;31m%s\033[0m  %s  发送群消息：\n\t\033[;32m%s\033[0m", message.sendName , message.msgTime , message.content);
				printf("%s\n", str);
				break;
			
			case PERSONAL_CHAT:
				sprintf(str , "\033[;31m%s\033[0m  %s  向你发送私聊消息：\n\t\033[;32m%s\033[0m", message.sendName , message.msgTime , message.content);
				printf("%s\n\n", str);
				break;
			
			case PERSONAL:
				personalRet = message.msgRet;
				break;
				
			case RESULT:
				printf("你的操作结果：%s\n", message.content);
				break;
				
			case EXIT:
				printf("已退出登录！\n");
				return;
				
			default:
				break ; 
		}//switch
	}//while	
}

