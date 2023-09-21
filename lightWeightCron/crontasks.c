#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>

#include "ccronexpr.h"
#include "crontasks.h"

//#include "timeout_wrapper.h"

static unsigned int regnum = 1;
static struct cron_task *the_tasks = NULL;
pthread_t th_crond;

static struct itimerval one_timer;
struct timers
{
    int interval; //定时时间
    void(*handler)(cron_task_t *s); //处理函数
    cron_task_t *data;//参数
};

static struct timers cron_timer;

volatile int invoke_timeout = 0;


void
cron_task_unregister(unsigned int clientreg){
    struct cron_task *sa_ptr, **prevNext = &the_tasks;

    for (sa_ptr = the_tasks;
         sa_ptr != NULL && sa_ptr->clientreg != clientreg;
         sa_ptr = sa_ptr->next) {
        prevNext = &(sa_ptr->next);
        }
    if (sa_ptr != NULL) {
        *prevNext = sa_ptr->next;
        /*
         * Note: do not free the clientarg, it's the client's responsibility 
         */
        free(sa_ptr);
    }
}
void
cron_task_unregister_all(void)
{
  struct cron_task *sa_ptr, *sa_tmp;

  for (sa_ptr = the_tasks; sa_ptr != NULL; sa_ptr = sa_tmp) {
    sa_tmp = sa_ptr->next;
	free(sa_ptr->expr);
    free(sa_ptr);
  }
  the_tasks = NULL;
}  
					
unsigned int
cron_task_register(char* when,char* task_name,
					CronCallback * thecallback, void *clientarg,unsigned long time_out)
{	
	/*任务链表*/
	struct cron_task **s = NULL;
	
	for (s = &(the_tasks); *s != NULL; s = &((*s)->next));

	*s = CRON_MALLOC_STRUCT(cron_task);
	if (*s == NULL) {
		return 0;
	}
	(*s)->schedule = when;
	(*s)->clientarg = clientarg;
	(*s)->thecallback = thecallback;
	(*s)->clientreg = regnum++;
	(*s)->cl_Pid = JOB_WAITING;
	(*s)->job_name=task_name;
	/*解析cron表达式，得到下次运行时间*/
	cron_expr *expr_s = CRON_MALLOC_STRUCT(cron_expr);
	const char* err = NULL;
	cron_parse_expr((*s)->schedule, expr_s, &err);
	 if (err) {
		perror("error parsing cron expression:");
		return 1;
	}
	(*s)->expr=expr_s;
	(*s)->next = NULL;
	(*s)->cl_Pid=JOB_WAITING;
	(*s)->timeout=time_out;
	/*更新nextTrigger*/
	updateNextTrigger((*s));
	return (*s)->clientreg;
}
struct cron_task *sa_ptr_global, *sa_tmp_global;
int  arm_jobs(void){
	short nJobs = 0;
	for (sa_ptr_global = the_tasks; sa_ptr_global != NULL; sa_ptr_global = sa_tmp_global) {
		time_t current_time = time(NULL);
		//printf("current_time=%d,nextTrigger=%d,arm_jobs =%s\n",current_time,sa_ptr->nextTrigger,sa_ptr->job_name);
		if(sa_ptr_global->nextTrigger<=current_time&&sa_ptr_global->cl_Pid!=JOB_ARMED){
			sa_ptr_global->cl_Pid=JOB_ARMED;
			//printf("arm_jobs =%s \n",sa_ptr->job_name);
			nJobs+=1;
			cron_set_timer(sa_ptr_global->timeout,sa_ptr_global);
			run_job(sa_ptr_global);
			cron_stop_timer();
		}
		sa_tmp_global = sa_ptr_global->next;
	}
	return nJobs;
}

void sighandler(int signum)
{
	if(signum==SIGUSR1){
		printf("Pthread of cron stop singal.\n");
		pthread_exit(0);
	}else if(signum==SIGALRM){
		printf("\n---");
		if(cron_timer.interval>0){
			cron_timer.interval--;
			if(!cron_timer.interval)
			{	
				(*(cron_timer.handler))(cron_timer.data);
			}
		}
	}
}
void timeout_handler(cron_task_t *s){
	if(s){
		//longjmp(invoke_env, 1);//goto next task;
		invoke_timeout=1
		printf("%s timeout,todo something\n",s->job_name);
	}
}
void *crond(){
	for (;;) {
		sleep(1);
		printf("wakeup\n");
		fflush(stdout);
		arm_jobs();
	}
}

void cron_run(){
	one_timer.it_interval.tv_sec=1; //设置单位定时器定时时间
    one_timer.it_value.tv_sec=1; //设置单位定时器初始值
    setitimer(ITIMER_REAL,&one_timer,NULL); //初始化单位定时器
    signal(SIGALRM,sighandler); //指定单位定时器定时时间到时执行的函数
	
	signal(SIGUSR1, sighandler);
    pthread_create(&th_crond, NULL, crond, NULL); 
    pthread_detach(th_crond);
}

void cron_stop(void){
	int rc = pthread_kill(th_crond, SIGUSR1);
}

int run_job(void   * param)

{	
	cron_task_t *s=(cron_task_t*)param;
	if(s->cl_Pid==JOB_ARMED)
	{
		s->cl_Pid=JOB_NONE;
		(*((s)->thecallback)) ((s)->clientreg, (s)->clientarg);
		prev_stamp(s);
		updateNextTrigger(s);
		s->cl_Pid=JOB_WAITING;
	}
	return 0;
}
void globalUpdateNextTrigger(void){
	struct cron_task *sa_ptr, *sa_tmp;
	for (sa_ptr = the_tasks; sa_ptr != NULL; sa_ptr = sa_tmp) {
		updateNextTrigger(sa_ptr);
		sa_tmp = sa_ptr->next;
	}
}
void updateNextTrigger(cron_task_t* task){
	time_t current_time = time(NULL);
    time_t next_run;
	next_run = cron_next(task->expr, current_time);
	task->nextTrigger=next_run;
}
void prev_stamp(cron_task_t* task){
	time_t current_time = time(NULL);
	task->cl_NotUntil=cron_prev(task->expr, current_time);
}

static void cron_stop_timer() 
{
	cron_timer.interval = 0;
}

static int cron_set_timer(unsigned long timeout,cron_task_t *s)
{
	memset(&cron_timer , 0 , sizeof(cron_timer));
	cron_timer.interval = timeout;
	cron_timer.handler=timeout_handler;
	cron_timer.data=s;
	return 0;
}

/*timeout experiment*/
sigjmp_buf invoke_env;

void 
timeout_signal_handler(int sig) 
{
	if(sig==SIGALRM){
		invoke_count++;
    	siglongjmp(invoke_env, 1);
	}else if(sig==SIGUSR2){
		printf("超时操作");
	}
}

sigfunc *
my_signal(int signo, sigfunc *func)
{
    struct sigaction act, oact;

    act.sa_handler = func;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    if (signo == SIGALRM) {
#ifdef SA_INTERRUPT
        act.sa_flags |= SA_INTERRUPT;
#endif
    } else {
#ifdef SA_RESTART
        act.sa_flags |= SA_RESTART;
#endif
    }
    if (sigaction(signo, &act, &oact) < 0)
        return SIG_ERR;
    return oact.sa_handler;
}

