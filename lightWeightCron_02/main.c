/*************************************************
Author:zhouBL
Version:
Description:
Others:
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
static unsigned int thid;
void func(unsigned int clientreg,void *clientarg){ 
	time_t t1=time(NULL) ;
    printf("This is func.str = %s current time is %s \n", (char*)(clientarg),ctime(&t1)); 
}
void func2(unsigned int clientreg,void *clientarg){ 
	time_t t1 = time(NULL);
    printf("This is func2.str = %s current time is %s \n", (char*)(clientarg),ctime(&t1)); 
}

void sig_handler(int signo) {
    if (signo == SIGINT) {
        cron_callback_unregister_all();
        _exit(0);
    }
}
// int main(int argc, char *argv[]) {
//     signal(SIGINT, sig_handler);
//     thid=cron_callback_register("01 * * * *",func, "aaa");
//     thid=cron_callback_register("02 * * * *",func2, "bbb");
//     while(1);
//     return EXIT_SUCCESS;
// }

void *crond(){
    for (;;) {
		time_t current_time = time(NULL);
		if(the_tasks!=NULL){
		sleep(current_time-next_runtime);
		run_job_temp;
	}
    }
}
int main(int argc, char *argv[]) {
    signal(SIGINT, sig_handler);
    pthread_t th_crond;
    //pthread_create(&th_crond, NULL, crond, NULL); 
    pthread_detach(th_crond);

    thid=cron_callback_register("*/5 * * * * *","task1",func, "aaa");
    thid=cron_callback_register("*/2 * * * * *","task2",func2, "bbb");
    while(1);
	return EXIT_SUCCESS;
}