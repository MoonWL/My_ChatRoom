/*******************************************************
 * 服务器基本配置文件 --包含所需头文件
 * 用户信息结构体定义
 * 在线用户链表定义
 * 2016-4-25 wl实现
********************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include<memory.h>

#include<time.h>
#include<unistd.h>
#include<errno.h>
#include<fcntl.h>

#include<sys/ipc.h>
#include<sys/msg.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<sys/select.h>
#include<sys/time.h>
#include<pthread.h>

#include<sqlite3.h>

/*FD_SETSIZE定义描述符集的大小，定义在sys/types.h中*/
#ifndef FD_SETSIZE
#define FD_SETSIZE 256
#endif

#define MAX_LINE 8192
#define PORT 8889//服务器的端口号
#define LISTENEQ 6000 //允许连接的最大数

/*预定义数据库名称*/
#define DB_NAME "../ChatRoom.db"

/*禁言标志*/
enum Flag{
	YES,
	No
};

/*定义C/S的消息类型*/
enum MessageType{
	REGISTER = 1,//注册请求
	LOGIN,	//登录请求
	HELP,	//帮助请求
	EXIT,	//退出请求
	VIEW_USER_LIST,	//查看在线用户列表
	GROUP_CHAT,	//群聊请求
	PERSONAL_CHAT,	//私聊请求
	PERSONAL,	//私聊对象请求
	VIEW_RECORDS,		//查看聊天记录请求
	RESULT,	//结果消息类型
	UNKNOW	//未知请求类型
};

/*定义操作结果*/
enum StateRet{
	EXCEED,	//已达服务器连接上限
	SUCCESS,	//成功
	FAILED,	//失败
	DUPLICATEID,	//用户名已存在
	INVALID,	//不合法的用户名
	ID_NOT_EXIST,	//帐号不存在
	WRONGPWD,	//密码错误
	ALREADY_ONLINE,	//已经在线
	ID_NOT_ONLINE,	//帐号不在线
	ALL_NOT_ONLINE,	//无人在线
	MESSAGE_SELF	//消息对象不能选择自己
};

/*定义C/S消息结构体*/
typedef struct _Message{
	char content[2048];	//针对聊天类型的消息，填充该字段*/
	int msgType;	//消息类型，即为MessageType中的值
	int msgRet;	//针对操作结果类型的消息，填充该字段
	struct sockaddr_in sendAddr;	//消息发送者的套接字地址
	struct sockaddr_in recvAddr;	//消息接收者的套接字地址
	char sendName[20];	//消息发送者的名称
	char recvName[20];	//消息接收者的名称
	char msgTime[20];	//消息发送时间
}Message;

/*用户信息结构体*/
typedef struct _User{
	char userName[20];	//用户名
	char password[20];//密码
	struct sockaddr_in userAddr;	//用户登录的套接字地址
	int sockfd;	//当前用户使用的套接字描述符
	int speak;	//禁言标志
	char registerTime[20];	//用户注册时间
}User;

/*定义用户链表结构体*/
typedef struct _ListNode{
	User user;
	struct _ListNode *next;
}ListNode;

/*定义在线用户链表*/
extern ListNode *userList;
//定义互斥访问数据库
extern pthread_mutex_t lock1;
//定义互斥访问在线用户链表
extern pthread_mutex_t lock2;

/*server.c中客户请求处理函数*/
extern void *handleRequest(int *fd);

/*config.c文件函数声明*/
extern char *stateMsg(int stateRet);
extern void copyUser(User *user1,User *user2);
ssize_t writen(int fd, const void *ptr, size_t n);
ssize_t readn(int fd, void *ptr, size_t n);

/*chat.c文件函数声明*/
extern int enterChat(int *fd);
//extern int groupChat(Message *msg,int sockfd);
//extern int personalChat(Message &msg,int sockfd);
extern int viewUserList(Message *msg,int sockfd);
//extern int viewRecords(Message *msg,int sockfd);

/*list.c文件函数声明*/
extern ListNode* insertNode(ListNode *list,User *user);
extern int isOnLine(ListNode *list,User *user);
extern ListNode* deleteNode(ListNode *list,User *user);
extern void displayList(ListNode *list);

/*login.c文件函数声明*/
extern int loginUser(Message *msg,int sockfd,User *u);

/*register.c文件函数声明*/
extern int registerUser(Message *msg,int sockfd);

