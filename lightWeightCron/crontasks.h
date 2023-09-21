#ifndef CRONTASKS_H
#define CRONTASKS_H
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
		unsigned long 		 	  timeout;
        struct cron_task *next;
    }cron_task_t;



unsigned int 
cron_task_register(char* when,char* task_name,
                    CronCallback * thecallback, void *clientarg
                    ,unsigned long timeout);
void cron_task_unregister(unsigned int clientreg);

int run_job(void   * param);

void cron_run();
void cron_stop(void);
void *crond();
void sighandler(int signum);
void prev_stamp(cron_task_t* task);

int  arm_jobs(void);

void cron_task_unregister_all(void);

void updateNextTrigger(cron_task_t* task);
void globalUpdateNextTrigger(void);
/*timeout experiment*/
static void cron_stop_timer() ;
static int cron_set_timer(unsigned long timeout,cron_task_t* s);
void timeout_handler(cron_task_t *s);
extern sigjmp_buf invoke_env;
extern volatile int invoke_timeout;

typedef void sigfunc(int sig);
sigfunc *my_signal(int signo, sigfunc* func);
#define E_CALL_TIMEOUT (-9)

/* interval: microseconds */
#define add_timeout_to_func(func, interval, ret,...) \
    { \
        invoke_timeout = 0; \
        sigfunc *sf = my_signal(SIGALRM, sighandler); \
        if (sf == SIG_ERR) { \
            ret = errno; \
            goto end; \
        }  \
\
        if (sigsetjmp(invoke_env, SIGALRM) != 0) { \
            if (invoke_timeout) { \
                ret = E_CALL_TIMEOUT; \
                goto err; \
            } \
        } \
\
        struct itimerval tick;  \
        struct itimerval oldtick;  \
        tick.it_value.tv_sec = interval/1000; \
        tick.it_value.tv_usec = (interval%1000) * 1000; \
        tick.it_interval.tv_sec = interval/1000; \
        tick.it_interval.tv_usec = (interval%1000) * 1000; \
\
        if (setitimer(ITIMER_REAL, &tick, &oldtick) < 0) { \
            ret = errno; \
            goto err; \
        } \
\
        ret=func(__VA_ARGS__);\
        setitimer(ITIMER_REAL, &oldtick, NULL); \
err:\
		my_signal(SIGUSR2, sf);\
		/*printf("todo something\n")*/;\
end:\
        ;\
    }

#endif 
