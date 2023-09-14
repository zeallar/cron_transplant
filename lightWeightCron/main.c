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
time_t t1 = time(NULL);
    time_t t2;
    long dt;
    short stime = 60;

    for (;;) {
        sleep((stime + 1) - (short)(time(NULL) % stime));

        t2 = time(NULL);
        dt = t2 - t1;
		printf("Wakeup dt=%d\n", dt);
        if (dt < -60*60 || dt > 60*60) {
            t1 = t2;
        }else if (dt > 0) {
            arm_job(t1, t2);
            run_jobs();
            sleep(5);
            if (check_running_jobs() > 0)
					stime = 10;
				else
					stime = 60;
            t1 = t2;
        }
    }
}
int main(int argc, char *argv[]) {
    signal(SIGINT, sig_handler);
    pthread_t th_crond;
    pthread_create(&th_crond, NULL, crond, NULL); 
    pthread_detach(th_crond);


    thid=cron_callback_register("* * * * *","task1",func, "aaa");
    thid=cron_callback_register("*/2 * * * *","task2",func2, "bbb");
    while(1);
	return EXIT_SUCCESS;
}