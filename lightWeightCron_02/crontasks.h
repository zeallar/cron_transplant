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
        /*上次运行时间*/
        struct cron_task *next;
    }cron_task_t;
static struct cron_task *the_tasks = NULL;
static int next_runtime=0;

unsigned int 
cron_callback_register(char* when,char* task_name,
                    CronCallback * thecallback, void *clientarg);
void cron_callback_unregister(unsigned int clientreg);
int nap(cron_task_t *job);
void run_job_temp(void *param);
void cron_callback_unregister_all(void);

void updateNextTrigger(cron_task_t* task);
void globalUpdateNextTrigger(void);
struct cron_task* toSortList(struct cron_task* head, struct cron_task* tail) ;
struct cron_task* merge(struct cron_task* head1, struct cron_task* head2);



#endif 
