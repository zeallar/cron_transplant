#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <setjmp.h>

#include "ccronexpr.h"
#include "crontasks.h"

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
sigjmp_buf invoke_env;


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
		return -1;
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
		return -1;
	}
	(*s)->expr=expr_s;
	(*s)->next = NULL;
	(*s)->cl_Pid=JOB_WAITING;
	(*s)->timeout=time_out;
	/*更新nextTrigger*/
	updateNextTrigger((*s));
	return (*s)->clientreg;
}

int  arm_jobs(void){
	short nJobs = 0;
	struct cron_task *sa_ptr_global, *sa_tmp_global;
	for (sa_ptr_global = the_tasks; sa_ptr_global != NULL; sa_ptr_global = sa_tmp_global) {
		volatile time_t current_time = time(NULL);
		//printf("current_time=%d,nextTrigger=%d,arm_jobs =%s\n",current_time,sa_ptr->nextTrigger,sa_ptr->job_name);
		if(sa_ptr_global->nextTrigger<=current_time&&sa_ptr_global->cl_Pid!=JOB_ARMED){
			sa_ptr_global->cl_Pid=JOB_ARMED;
			//printf("arm_jobs =%s \n",sa_ptr->job_name);
			nJobs+=1;
			if (sigsetjmp(invoke_env, SIGALRM)) {
				goto jump;
			}
			cron_set_timer(sa_ptr_global->timeout,sa_ptr_global);
			run_job(sa_ptr_global);
			cron_stop_timer();
		}
		sa_tmp_global = sa_ptr_global->next;
		jump:
			sa_tmp_global = sa_ptr_global->next;
		
	}	
	return nJobs;
}


void sighandler(int signum)
{
	if(signum==SIGALRM){
		//printf("\n---");
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
		printf("%s timeout,todo something\n",s->job_name);
		s->timeout_record=time(NULL);//record timeout time 
		siglongjmp(invoke_env, 1);//goto next task;
		}
}
void* crond(){
	one_timer.it_interval.tv_sec=1; //设置单位定时器定时时间
	one_timer.it_value.tv_sec=1; //设置单位定时器初始值
	setitimer(ITIMER_REAL,&one_timer,NULL); //初始化单位定时器
	my_signal(SIGALRM,sighandler);//指定单位定时器定时时间到时执行的函数
	
	for (;;) {
		sleep(1);
		//printf("wakeup\n");
		arm_jobs();
	}
}
void 
my_signal(int signo, void *func)
{
    struct sigaction act;

    act.sa_handler = func;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(signo, &act, NULL);
 
}

void cron_run(){
   int err = 0;
	pthread_t th;
	/*以分离状态启动子线程*/
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if((err = pthread_create(&th, &attr, crond, NULL) != 0)){
        perror("pthread_create error");
    }
}

int run_job(void   * param)

{	
	cron_task_t *s=(cron_task_t*)param;
	if(s->cl_Pid==JOB_ARMED)
	{
		s->cl_Pid=JOB_NONE;
		//printf("next triggertime=%s\n",ctime(&(s->nextTrigger)));
		(*((s)->thecallback)) ((s)->clientreg, (s)->clientarg);
		prev_stamp(s);
		updateNextTrigger(s);
		s->cl_Pid=JOB_WAITING;
	}
	return 0;
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
