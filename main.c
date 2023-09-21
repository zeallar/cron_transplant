/*************************************************
Author:zhouBL
Version:
Description:cron lib
Others:1、任务超时功能;2、日志记录执行任务执行结果;3、50个以上任务测试。
created date:2023/9/9 2:32 下午
modified date:2023/9/21 4:31 下午
*************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>

#include "crontasks.h"

void func(unsigned int clientreg,void *clientarg){ 
	time_t t1=time(NULL) ;
    printf("This is func.str = %s current time is %s \n", (char*)(clientarg),ctime(&t1)); 
}
void func2(unsigned int clientreg,void *clientarg){ 
	time_t t1 = time(NULL);
	int i=0;
	for(;i<10000000000;i++);//回调函数用sleep会被cron子线程SIGALRM信号打断
    printf("This is func2.str = %s current time is %s \n", (char*)(clientarg),ctime(&t1)); 
}
void func3(unsigned int clientreg,void *clientarg){ 
	time_t t1=time(NULL) ;
    printf("This is func3.str = %s current time is %s \n", (char*)(clientarg),ctime(&t1)); 
}


int app_running=1;
void sig_handler(int signo) {
    if (signo == SIGINT) {
        cron_task_unregister_all();
        app_running=0;
    }
}
int main(int argc, char *argv[]) {
	

    my_signal(SIGINT, sig_handler);
    cron_task_register("*/30 * * * * ?","task1",func, "aaa",2);
    cron_task_register("*/6 * * * * *","task2",func2, "bbb1",2);
	cron_task_register("*/12 * * * * *","task2",func3, "bbb2",2);
	//cron_task_register("*/1 * * * * *","task2",func2, "bbb3",2);
	//cron_task_register("*/1 * * * * *","task2",func2, "bbb4",2);
    cron_run();
    
	/*主线程屏蔽SIGALRM信号，由cron子线程处理*/
	sigset_t set;
	int s;
	sigemptyset(&set);
	sigaddset(&set, SIGUSR1);
	sigaddset(&set, SIGALRM);
	s = pthread_sigmask(SIG_SETMASK, &set, NULL);
	if (s != 0)
       printf("pthread_sigmask error\n");
    while(app_running);
	printf("main thread ends\n");
	return EXIT_SUCCESS;
}
