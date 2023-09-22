/*************************************************
Author:zhouBL
Version:
Description:cron lib
Others:测试
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
    printf("This is func.\t str = %s current time is %s \n", (char*)(clientarg),ctime(&t1)); 
}
void func2(unsigned int clientreg,void *clientarg){ 
	time_t t1 = time(NULL);
	int i=0;
	for(;i<10000000000;i++);//回调函数用sleep会被cron子线程SIGALRM信号打断
    printf("This is func2.\t str = %s current time is %s \n", (char*)(clientarg),ctime(&t1)); 
}
void func3(unsigned int clientreg,void *clientarg){ 
	time_t t1=time(NULL) ;
    printf("This is func3.\t str = %s current time is %s \n", (char*)(clientarg),ctime(&t1)); 
}


int app_running=1;
void sig_handler(int signo) {
    if (signo == SIGINT) {
        cron_task_unregister_all();
        app_running=0;
    }
}
int main(int argc, char *argv[]) {
	
	int reID;
    my_signal(SIGINT, sig_handler);
    reID=cron_task_register("*/24 * * * * ?","task1",func, "aaa",2);
    reID=cron_task_register("*/6 * * * * *","task2",func2, "bbb",2);
	reID=cron_task_register("*/12 * * * * *","task3",func3, "ccc",2);
	
	reID=cron_task_register("0 0/3 * * * ?","task4",func3, "heartbeat",2);
	reID=cron_task_register("0 0/5 * * * ?","task5",func3, "gwstatus",2);
	reID=cron_task_register("0 0/30 * * * ?","task6",func3, "gatewayevents",2);
	reID=cron_task_register("0 0/30 * * * ?","task7",func3, "waterenergy",2);
	reID=cron_task_register("0 0/10 * * * ?","task8",func3, "wateralarms",2);
	reID=cron_task_register("0 0/10 * * * ?","task9",func3, "waterevents",2);
	reID=cron_task_register("0 0/20 * * * ?","task10",func3, "waterbilling",2);
	reID=cron_task_register("0 0/10 * * * ?","task11",func3, "alarms",2);
	reID=cron_task_register("0 0 * * * ?","task12",func3, "events",2);
	reID=cron_task_register("0 0/20 * * * ?","task13",func3, "energyprofile",2);
	reID=cron_task_register("0 0/30 * * * ?","task14",func3, "instantaneousprofile",2);
	reID=cron_task_register("0 0,25 * * * ?","task15",func3, "maxdemandprofile",2);
	reID=cron_task_register("0 0 * * * ?","task16",func3, "loadprofile1",2);
	reID=cron_task_register("0 0 * * * ?","task11",func3, "loadprofile2",2);
	reID=cron_task_register("0 0 * * * ?","task12",func3, "powerqualityprofile",2);
	reID=cron_task_register("0 0 * * * ?","task13",func3, "instrumentationprofile",2);
	reID=cron_task_register("0 1 * * * ?","task15",func3, "billingprofile",2);
	reID=cron_task_register("0 0 * * * ?","task16",func3, "InstantaneousSave",2);
	reID=cron_task_register("0 0 * * * ?","task12",func3, "EnergySave",2);
	reID=cron_task_register("0 0 0 * * ?","task13",func3, "MaxdemandSave",2);
	reID=cron_task_register("0 0 2 * * ?","task15",func3, "WaterSave",2);
	if(reID<0){
		perror("Task registration error:");
		app_running=0;
	}
    cron_run();
    
	/*主线程屏蔽SIGALRM信号，由cron子线程处理*/
	sigset_t set;
	int s;
	sigemptyset(&set);
	sigaddset(&set, SIGALRM);
	s = pthread_sigmask(SIG_SETMASK, &set, NULL);
	if (s != 0)
       printf("pthread_sigmask error\n");
    while(app_running);
	printf("main thread ends\n");
	return EXIT_SUCCESS;
}
