/*************************************************
Author:zhouBL
Version:
Description:
Others:1、任务超时功能;2、日志记录执行任务执行结果;3、50个以上任务测试。
created date:2023/9/9 2:32 下午
modified date:
*************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>

#include "crontasks.h"
#include "timeout_wrapper.h"

void func(unsigned int clientreg,void *clientarg){ 
	time_t t1=time(NULL) ;
    printf("This is func.str = %s current time is %s \n", (char*)(clientarg),ctime(&t1)); 
}
void func2(unsigned int clientreg,void *clientarg){ 
	time_t t1 = time(NULL);
	sleep(2);
    printf("This is func2.str = %s current time is %s \n", (char*)(clientarg),ctime(&t1)); 
}


void sig_handler(int signo) {
    if (signo == SIGINT) {
        cron_task_unregister_all();
		cron_stop();
        _exit(0);
    }
}
int main(int argc, char *argv[]) {


    signal(SIGINT, sig_handler);
    cron_task_register("*/30 * * * * ?","task1",func, "aaa",2);
    cron_task_register("*/6 * * * * *","task2",func2, "bbb1",2);
	//cron_task_register("*/1 * * * * *","task2",func2, "bbb2",2);
	//cron_task_register("*/1 * * * * *","task2",func2, "bbb3",2);
	//cron_task_register("*/1 * * * * *","task2",func2, "bbb4",2);
    cron_run();
    while(1);
	return EXIT_SUCCESS;
}
