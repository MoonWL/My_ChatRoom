/********************************************
 * 服务器端代码实现server.c
 * 2016-4-25 wl实现
********************************************/

#include"config.h"

ListNode *userList = NULL;
pthread_mutex_t lock1;
pthread_mutex_t lock2;
/*********************************************
 * 函数名：main
 * 功能：聊天室服务器main函数入口
 * 参数：无
 * 返回值：正常退出返回0，否则返回1
***********************************************/

int main(void)
{
	/*声明服务器监听描述符和已连接描述符*/
	int i,n,ret,maxi,maxfd,listenfd,connfd,sockfd;
	
	socklen_t clilen;
	
	pthread_t pid;
	
	/*套接字选项，允许服务器重启时还是用原来的套接字*/
	int opt = 1;
	
	/*声明服务器地址和客户端地址结构 */
	struct sockaddr_in servaddr,cliaddr;
	
	/*声明描述符集*/
	fd_set rset,allset;
	/*nready为当前可用的描述符数量,client_sockfd数组存储已连接的套接字描述符*/
	int nready,client_sockfd[FD_SETSIZE];
	
	/*声明消息变量*/
	Message message;
	/*声明消息缓冲区*/
	char buf[MAX_LINE];
	
	/*用户信息*/
	User user;
	
	system("clear");
	printf("\033[;32m                        聊天服务器运行中            \033[0m\n\n");
	//printf("adadad%d\n",sizeof(message.sendAddr));
	/*(1)创建监听套接字*/
	if((listenfd = socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		perror("create socket error: ");
		printf("\n\n");
		exit(1);		
	}
	
	/*(2)初始化服务器的监听套接字地址结构*/
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);//8889
	
	/*(3)绑定监听套接字与其地址结构*/
	setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
	if(bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0)
	{
		perror("bind error: ");
		printf("\n\n");
		exit(1);
	}
	
	/*(4)开启监听*/
	if(listen(listenfd,LISTENEQ) < 0)
	{
		perror("listen error: ");
		printf("\n\n");
		exit(1);
	}
	
	/*(5)初始化已连接描述符数组*/
	maxfd = listenfd;//待测试的描述符个数
	maxi = -1;//client_sockfd数组的索引
	for(i = 0; i < FD_SETSIZE;i++)
	{
		client_sockfd[i] = -1;
	}
	/*初始化allset描述符集*/
	FD_ZERO(&allset);
	FD_SET(listenfd,&allset);
	
	/*(6)接收客户连接请求*/
	for(;;){
		rset = allset;//select返回后，所有未就绪的描述符对应的位均被清0，固保存原描述符集
		nready = select(maxfd+1,&rset,NULL,NULL,0);
		
		/*测试listenfd是否在rset中，即有连接到达listenfd，此处只处理连接*/
		if(FD_ISSET(listenfd,&rset))
		{
			/*接收客户端的连接请求，分配一个已连接描述符*/
			clilen = sizeof(cliaddr);
			if((connfd = accept(listenfd,(struct sockaddr *)&cliaddr,&clilen)) < 0)
			{
				perror("accepr error: ");
				printf("\n\n");
				exit(1);
			}
			
			printf("server: got connection from %s\n\n",inet_ntoa(cliaddr.sin_addr));
			
			/*查找空闲位置，存储已连接套接字描述符*/
			for(i = 0;i < FD_SETSIZE;i++)
			{
				if(client_sockfd[i] < 0){
					client_sockfd[i] = connfd;
					break;
				}
			}
			
			/*如果client_sockfd数组已满*/
			if(i == FD_SETSIZE)
			{
				perror("too many connection: ");
				printf("\n\n");
				exit(1);
			}
			
			/*更新client_sockfd数组的索引*/
			if(i > maxi)
				maxi = i;
			
			/*将已连接描述符加入描述符集*/
			FD_SET(connfd,&allset);
			
			/*新的描述符数目 -- for select*/
			if(connfd > maxfd)
				maxfd = connfd;
			
			/*如果就绪的套接字描述符数<=1,即只有监听套接字就绪，则说明无客户端的连接达到*/
			if( --nready <= 0)continue;
		}
		
		/*如果就绪的已连接套接字描述符数>=1,接下来逐个处理已连接的描述符,此处只处理数据*/
		for(i = 0;i <=maxi;i++)
		{
			if((sockfd = client_sockfd[i]) < 0)
				continue;
			if(FD_ISSET(sockfd,&rset))//sockfd上有数据到达
			{
				//创建一个线程处理数据传输
				pthread_create(&pid , NULL , (void *)handleRequest , (void *)&sockfd);
				//select不再监听sockfd,因为已交由线程处理数据传输
				FD_CLR(sockfd,&allset);
				client_sockfd[i] = -1;	
				if(--nready <= 0)break;//没有就绪的已连接描述符				
			}
			
			//对于无数据到达的已连接描述符，则下次select继续监听
		}
	}
	
	close(listenfd);
	return 0;
}
	
/*处理与客户传输数据的线程*/
void *handleRequest(int *fd)
{
	int sockfd,ret,n;
	
	/*声明消息变量*/
	Message message;
	
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
	}
	
	
	while(1){
		//接受用户发送的消息
		memset(buf,0,MAX_LINE);
		memset(&message,0,sizeof(message));
		n = readn(sockfd,buf,sizeof(message));
		if(n == sizeof(message)){
			//printf("已经接收了%d字节数据\n",n);
			//printf("消息类型%d\n",message.msgType);
		}
		if(n <= 0)
		{
			//关闭当前描述符，向客户端发送FIN
			fflush(stdout);
			close(sockfd);
			*fd = -1;
			//打印发送者的IP
			printf("来自%s的断开连接！\n\n",inet_ntoa(cliaddr.sin_addr));
			pthread_exit(NULL);//此线程退出
			return NULL;
		}
		
		memcpy(&message,buf,sizeof(message));
		message.sendAddr = cliaddr;
		
		//printf("消息类型%d\n",message.msgType);
		switch(message.msgType)
		{
			case REGISTER:{
				printf("\n来自%s注册申请！\n",inet_ntoa(message.sendAddr.sin_addr));
				ret = registerUser(&message,sockfd);//交由sqlite3数据库处理
				//if(ret == FAILED)break;
				memset(&message,0,sizeof(message));
				message.msgType = RESULT;
				message.msgRet = ret;
				printf("%s\n",stateMsg(ret));
				strcpy(message.content , stateMsg(ret));
				memset(buf,0,MAX_LINE);
				memcpy(buf,&message,sizeof(message));
				/*发送操作结果消息*/
				if(writen(sockfd,buf,sizeof(message)) < 0){
					perror("send error: ");
					printf("\n\n");
					exit(1);
				}
				printf("注册：%s\n", stateMsg(message.msgRet));
				break;
			}
			 case LOGIN:{
				User user;
				printf("来自%s的登录请求！\n",inet_ntoa(message.sendAddr.sin_addr));
				ret = loginUser(&message,sockfd,&user);//交由sqlite3数据库处理
				//if(ret == FAILED)break;//客户登录界面突然退出
				memset(&message,0,sizeof(message));
				message.msgType = RESULT;
				message.msgRet = ret;
				strcpy(message.content,stateMsg(ret));
				memset(buf,0,MAX_LINE);
				memcpy(buf,&message,sizeof(message));
				//发送操作结果消息
				if(writen(sockfd,buf,sizeof(message)) < 0){
					perror("send error: ");
					printf("\n\n");
					exit(1);
				}
				//printf("登录：%s\n",stateMsg(ret));
				if(ret == SUCCESS){
					//进入服务器聊天模式
					ret = enterChat(&sockfd);
					if(ret == FAILED){//客户enterChat界面突然退出
						printf("\033[;31m%s\033[0m退出登录\n",user.userName);
						userList = deleteNode(userList , &user);
						printf("来自%s的断开连接！\n\n",inet_ntoa(cliaddr.sin_addr));
						pthread_exit(NULL);//此线程退出
						return NULL;
					}
				}
				break;
			} 
			default:
				printf("unknown operator.\n");
				//close(sockfd);
				break;
		}
	}
	
	//关闭当前描述符，向客户端发送FIN
	close(sockfd);
	*fd = -1;
	return NULL;
}
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	

