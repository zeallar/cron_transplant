#ifndef CRONTASKS_H
#define CRONTASKS_H
#include <setjmp.h>

typedef void    (CronCallback) (unsigned int clientreg,
                                         void *clientarg);
#define JOB_NONE        0
#define JOB_ARMED       -1
#define JOB_WAITING     -2
#define CRON_MALLOC_STRUCT(s)   (struct s *) calloc(1, sizeof(struct s))

typedef struct cron_task {
        char*            schedule;/*执行时间，cron表达式*/
        char*             job_name;/*任务名称*/   
        unsigned int    clientreg;/*任务唯一注册id*/
        void           *clientarg;/*任务回调函数参数*/
        CronCallback *thecallback;/*回调函数*/
        struct cron_expr*    expr;/*解析后的cron表达式*/
		time_t 		  nextTrigger;
        int		           cl_Pid;/* running pid, 0, or armed (-1), or waiting (-2) */
        time_t 		  cl_NotUntil;/*最后运行时间*/
		unsigned long 	  timeout;
		time_t 	   timeout_record;/*任务执行超时时间*/
        struct cron_task *next;
    }cron_task_t;



unsigned int 
cron_task_register(char* when,char* task_name,
                    CronCallback * thecallback, void *clientarg
                    ,unsigned long timeout);
void cron_task_unregister(unsigned int clientreg);

int run_job(void   * param);

void cron_run();

void* crond();
void sighandler(int signum);
void prev_stamp(cron_task_t* task);

int  arm_jobs(void);

void cron_task_unregister_all(void);

void updateNextTrigger(cron_task_t* task);
/*timeout experiment*/
static void cron_stop_timer() ;
static int cron_set_timer(unsigned long timeout,cron_task_t* s);
void timeout_handler(cron_task_t *s);
void 
my_signal(int signo, void *func);


#endif 
