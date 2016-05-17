/***************************************************
 * 服务器在线用户链表结构与操作
 * 2016-5-5 wl实现
*****************************************************/

#include"config.h"

ListNode* insertNode(ListNode *list , User *user)
{
	pthread_mutex_lock(&lock2);
	/*建立新节点*/
	ListNode *node = (ListNode *)calloc(1, sizeof(ListNode));
	
	copyUser(&(node->user) , user);
	node->next = NULL;
	
	if(list == NULL)
	{			
		list = node;
		list->next = NULL;
	}//if
	else{
		node->next = list;
		list = node;
	}
	printf("更新在线用户列表！\n");
	pthread_mutex_unlock(&lock2);
	return list;
}

int isOnLine(ListNode *list , User *user)
{
	ListNode *p = list , *pre = p;
	while(p!=NULL && strcmp(p->user.userName , (*user).userName) != 0)
	{
		p = p->next;
	}//while
	
	/*不存在该在线用户*/
	if(p == NULL){
		//printf("无该用户！\n");
		return 0;
	}
		
	return 1;
}

ListNode* deleteNode(ListNode *list , User *user)
{
	pthread_mutex_lock(&lock2);
	if(list == NULL)
		return;

	ListNode *p = list , *pre = p;
	while(p!=NULL && strcmp(p->user.userName , (*user).userName) != 0)
	{
		pre = p;
		p = p->next;
	}//while
	
	/*不存在该在线用户*/
	if(p == NULL)
		return ;
	/*该用户位于链表头部*/
	else if(p == list)
	{
		//printf("在头部！\n");
		list = list->next;
	}//elif
	/*该用户位于链表尾部*/
	else if(p->next == NULL)
	{
		//printf("在尾部！\n");
		pre->next = NULL;
	}//elif
	/*该用户节点位于链表中间*/
	else
	{
		//printf("在中间！\n");
		pre->next = p->next;
	}//else
	/*释放该用户节点占用的空间*/
	free(p);
	pthread_mutex_unlock(&lock2);
	return list;
}

void displayList(ListNode *list)
{
	if(list == NULL)
		return;
	else
	{
		ListNode *p = list;
		while(p != NULL)
		{
			printf("\033[;32m%s --> \033[0m", p->user.userName);
			p = p->next;
		}
		printf("\n");
	}//else
}


