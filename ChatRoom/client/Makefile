MYNAME = makefile
CC = gcc -std=c11 -Wall -D_XOPEN_SOURCE

objects = client.o config.o register.o login.o interface.o chat.o

server: $(objects)
	$(CC) -g -o client $(objects)  -lpthread

client.o: client.c
	$(CC) -c client.c 

register.o: register.c
	$(CC) -c register.c

login.o: login.c
	$(CC) -c login.c

interface.o: interface.c
	$(CC) -c interface.c

chat.o: chat.c
	$(CC) -c chat.c

config.o: config.c
	$(CC) -c config.c
#比较稳健的clean做法，表示clean是一个伪目标
.PHONY: clean

#前面-的意思是：也许某些文件出现问题，忽略，继续执行
clean:
	-rm client $(objects) 

