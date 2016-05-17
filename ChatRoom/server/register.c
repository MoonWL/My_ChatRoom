/***********************************************************
 * 服务器处理用户注册实现文件
 * 2016-5-4 wl实现
***********************************************************/

#include"config.h"

/**********************************
 * 函数名：registerUser
 * 功能：用户注册函数实现
 * 参数：msg用户发送的注册请求消息，sockfd用于通信的套接字描述符
 * 返回值：成功注册返回SUCCESS,否则返回异常类型
***********************************/

int registerUser(Message *msg,int sockfd)
{
	int ret;
	
	User user;
	char buf[MAX_LINE];
	
	//声明数据库变量
	sqlite3 *db;
	sqlite3_stmt *stmt;
	const char *tail;
	
	//声明sql语句存储数据库操作语句
	char sql[256];
	
	//当前系统时间
	time_t timeNow;
	
	//存储操作结果的消息
	Message result,message;
	
	
	/*（1）打开数据库*/
	ret = sqlite3_open(DB_NAME, &db);
	if(ret != SQLITE_OK)
	{
		printf("unable open database.\n");
		return FAILED;
	}//if
	printf("Opened database successfully.\n");
	
	memset(buf,0,MAX_LINE);
	writen(sockfd,buf,sizeof(result));
	//printf("发送注册允许\n");
	
	//接受用户的注册信息：用户名，密码，端口号，禁言标志
	memset(buf,0,MAX_LINE);
	readn(sockfd,buf,sizeof(user));
	memset(&user,0,sizeof(user));
	memcpy(&user,buf,sizeof(user));
	user.userAddr  = (*msg).sendAddr;
	//printf("接受注册信息\n");
	if(strlen(user.userName) == 0 || strlen(user.password) == 0)
	{
		printf("空用户名和密码！\n");
		return FAILED;
	}	

	pthread_mutex_lock(&lock1);
	/*（2）检查要注册用户名是否已存在？*/
	memset(sql , 0 , sizeof(sql));
	sprintf(sql , "select * from User where userName='%s';",(user.userName));

	ret = sqlite3_prepare(db , sql , strlen(sql) , &stmt , &tail);	
	if(ret != SQLITE_OK)
	{
		ret = sqlite3_step(stmt);
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		printf("database select fail!\n");
		pthread_mutex_unlock(&lock1);
		return FAILED;
	}//if
	/*执行*/
	ret = sqlite3_step(stmt);
	//如果有数据则返回SQLITE_ROW，当到达末尾返回SQLITE_DONE
	 while (ret == SQLITE_ROW)
	 {
	     ret = sqlite3_step(stmt);
		 sqlite3_finalize(stmt);
		 sqlite3_close(db);
		 pthread_mutex_unlock(&lock1);
		 return DUPLICATEID;
	 }
	/*销毁句柄，关闭数据库*/
	sqlite3_finalize(stmt);

	/*执行插入操作*/
	memset(sql , 0 , sizeof(sql));
	time(&timeNow);
	//create table User(userName text primary key, password text, userAddr text, sockfd interger, speak interger, registerTime text);
	sprintf(sql , "insert into User(userName , password , userAddr , sockfd , speak , registerTime)\
			values('%s','%s','%s',%d, %d , '%s');",user.userName , user.password , 
			inet_ntoa(user.userAddr.sin_addr),user.sockfd , YES, asctime(gmtime(&timeNow)));

	ret = sqlite3_prepare(db , sql , strlen(sql) , &stmt , &tail);	
	if(ret != SQLITE_OK)
	{
		ret = sqlite3_step(stmt);
		sqlite3_finalize(stmt);
		sqlite3_close(db);
		pthread_mutex_unlock(&lock1);
		return FAILED;
	}//if
	
	/*顺利注册*/
	ret = sqlite3_step(stmt);
	sqlite3_finalize(stmt);
	sqlite3_close(db);
	pthread_mutex_unlock(&lock1);
	/*注册成功*/	
	return SUCCESS;
}


