#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include "cronexpress.h"
#include "crontasks.h"


static struct cron_task *the_tasks = NULL;
static unsigned int regnum = 1;

void
cron_callback_unregister(unsigned int clientreg){
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
cron_callback_unregister_all(void)
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
cron_callback_register(char* when,char* task_name,
                    CronCallback * thecallback, void *clientarg)
{
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
    (*s)->next = NULL;
	(*s)->job_name=task_name;
    /*解析cronexpress*/
    cron_expr *expr_s = CRON_MALLOC_STRUCT(cron_expr);;
    const char* err = NULL;
    cron_parse_expr((*s)->schedule, expr_s, &err);
    if (err) {
        perror("error parsing cron expression:");
        return 1;
    }
    (*s)->expr=expr_s;

    run_job_temp((*s));

    return (*s)->clientreg;
}
void run_job_temp(void *param)
{
    cron_task_t *s = (cron_task_t *)param;
    (*((s)->thecallback)) ((s)->clientreg, (s)->clientarg);

}

int
check_running_jobs(void){
    int nStillRunning = 0;
    struct cron_task *sa_ptr, *sa_tmp;
    for (sa_ptr = the_tasks; sa_ptr != NULL; sa_ptr = sa_tmp) {
        if (sa_ptr->cl_Pid == JOB_NONE) {
           nStillRunning+=1;
        }
        sa_tmp = sa_ptr->next;
    }
    return nStillRunning;
}
int
arm_job(time_t t1, time_t t2){
    short nJobs = 0;
    time_t t;
	printf("arm_job()%s:%d\n", __FILE__, __LINE__);
    /*
	 * Find jobs > t1 and <= t2
	 */

	for (t = t1 - t1 % 60; t <= t2; t += 60) {
		if (t > t1) {
			struct tm *tp = localtime(&t);

			char n_wday = 1 << ((tp->tm_mday - 1) / 7);
			if (n_wday >= FOURTH_DOW) {
				struct tm tnext = *tp;
				tnext.tm_mday += 7;
				if (mktime(&tnext) != (time_t)-1 && tnext.tm_mon != tp->tm_mon)
					n_wday |= LAST_DOW;	/* last dow in month is always recognized as 6th bit */
			}
            struct cron_task *sa_ptr, *sa_tmp;

            for (sa_ptr = the_tasks; sa_ptr != NULL; sa_ptr = sa_tmp) {
                sa_tmp = sa_ptr->next;
				if ((sa_ptr->cl_Pid == JOB_WAITING || sa_ptr->cl_Pid == JOB_NONE)) {
		            /* (re)schedule job? */
		            cron_expr* line=(cron_expr*)sa_ptr->expr;
		            if (line->cl_Mins[tp->tm_min] &&
		                    line->cl_Hrs[tp->tm_hour] &&
		                    (line->cl_Days[tp->tm_mday] && n_wday & line->cl_Dow[tp->tm_wday])
		                ) {
		                //nJobs += preparation_job(sa_ptr, t1, t2);
		                if (sa_ptr->cl_Pid != JOB_ARMED) {
		                	sa_ptr->cl_Pid=JOB_ARMED;
							nJobs ++;
		                }
		            }
				}
            }
                
            }

		}
	return(nJobs);

}

void
run_jobs(void){
    struct cron_task *sa_ptr, *sa_tmp;
    for (sa_ptr = the_tasks; sa_ptr != NULL; sa_ptr = sa_tmp) {
        if (sa_ptr->cl_Pid == JOB_ARMED) {
            (*((sa_ptr)->thecallback)) ((sa_ptr)->clientreg, (sa_ptr)->clientarg);
            sa_ptr->cl_Pid=JOB_WAITING;
        }
        sa_tmp = sa_ptr->next;
    }
}

