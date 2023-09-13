#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

#include "croncallback.h"
#include "ccronexpr.h"

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
    free(sa_ptr);
  }
  the_tasks = NULL;
}  
unsigned int
cron_callback_register(char* when,
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
    (*s)->next = NULL;
    
    pthread_t thrd;
    pthread_create(&thrd, NULL, run_job, (void *)(*s)); 
    pthread_detach(thrd);

    return (*s)->clientreg;
}
void *run_job(void *param)
{
    cron_task_t *s = (cron_task_t *)param;
    while (s) {
        if (nap(s)) {
            perror("error creating job");
            break;
        }
        (*((s)->thecallback)) ((s)->clientreg, (s)->clientarg);
    }
    pthread_exit(NULL);
}

int nap(cron_task_t *job) {
    time_t current_time = time(NULL);
    time_t next_run;
    cron_expr expr;
    const char* err = NULL;
    cron_parse_expr(job->schedule, &expr, &err);
    if (err) {
        perror("error parsing cron expression:");
        return 1;
    }
    next_run = cron_next(&expr, current_time);
    int sleep_duration = next_run - current_time;
    sleep(sleep_duration);
    return 0;
}