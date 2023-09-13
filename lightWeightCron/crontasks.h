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
        void*                expr;/*解析后的cron表达式*/
        int		           cl_Pid;/* running pid, 0, or armed (-1), or waiting (-2) */
        /*上次运行时间*/
        struct cron_task *next;
    }cron_task_t;

unsigned int 
cron_callback_register(char* when,
                    CronCallback * thecallback, void *clientarg);
void cron_callback_unregister(unsigned int clientreg);
int nap(cron_task_t *job);
void run_job_temp(void *param);
void cron_callback_unregister_all(void);
/*check for job completion
*1、检查当前正在运行的job数量
*/
int
check_running_jobs(void);

/*1、检查执行时间位于t1和t2之间的任务；
 *2、装载到等待列表
 */
int
arm_job(time_t t1, time_t t2);
/*1、读取所有需要执行的任务
 *2、执行任务
*/
void
run_jobs(void);

#endif 
