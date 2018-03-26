﻿
#ifndef _MAIN_H_
#define _MAIN_H_

#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include "Client.h"

void exceptionalStack(int signal);

int main( int argc, char **argv )
{
	/*捕获异常信息 start*/
	signal(SIGABRT, &exceptionalStack); //异常终止(abort)
	signal(SIGBUS,  &exceptionalStack); //硬件故障
	signal(SIGFPE,  &exceptionalStack); //算术异常
	signal(SIGILL,  &exceptionalStack); //非法硬件指令
	signal(SIGIOT,  &exceptionalStack); //硬件故障
	signal(SIGQUIT, &exceptionalStack); //终端退出符
	signal(SIGSEGV, &exceptionalStack); //无效存储访问
	signal(SIGSYS,  &exceptionalStack); //无效系统调用
	signal(SIGTRAP, &exceptionalStack); //硬件故障
	signal(SIGXCPU, &exceptionalStack); //超过CPU限制(setrlimit)
	signal(SIGXFSZ, &exceptionalStack); //超过文件长度限制(setrlimit)
	/*捕获异常信息 end*/

	setvbuf(stdout,NULL,_IONBF,0);  // 打印printf

	Client::createInstance();
	Client::getInstance()->init("../../server/bin/config/config.xml");

	Client::getInstance()->run();

	getchar();
	
	Client::destroyInstance();

	return 0;
}

void exceptionalStack(int signal)
{
	time_t nowtime;
	time(&nowtime);
	pid_t pid = getpid();
	char cmd[128];
	sprintf(cmd, "gstack %d > core_platform_gateway_log_%d.core", pid, nowtime);
	system(cmd);

	Client::getInstance()->onExit();
	exit( -1 );
}

#endif