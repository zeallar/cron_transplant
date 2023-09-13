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
#include <signal.h>

#include "croncallback.h"
static unsigned int thid;
void func(unsigned int clientreg,void *clientarg){ 
    printf("This is func.\t  b = %s\n", (char*)(clientarg)); 
}
void func2(unsigned int clientreg,void *clientarg){ 
    printf("This is func2.\t  b = %s\n", (char*)(clientarg)); 
}

void sig_handler(int signo) {
    if (signo == SIGINT) {
        cron_callback_unregister_all();
        _exit(0);
    }
}
int main(int argc, char *argv[]) {
    signal(SIGINT, sig_handler);
    thid=cron_callback_register("*/5 * * * * * *",func, "aaa");
    thid=cron_callback_register("*/5 * * * * * *",func2, "bbb");
    while(1);
    return EXIT_SUCCESS;
}
