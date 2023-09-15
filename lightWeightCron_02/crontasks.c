#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <string.h>


#include "ccronexpr.h"
#include "crontasks.h"


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
	time_t current_time = time(NULL);
	time_t next_run;
	next_run = cron_next(expr_s, current_time);
	(*s)->expr=expr_s;
	(*s)->nextTrigger=next_run;
	(*s)->next = NULL;
	
	/*更新nextTrigger*/
	globalUpdateNextTrigger();
	the_tasks=toSortList(the_tasks,(*s));
	next_runtime=the_tasks->nextTrigger;
	//run_job_temp((*s));
	return (*s)->clientreg;
}
					
void run_job_temp(void *param)
{
	struct cron_task *sa_ptr, *sa_tmp;
	for (sa_ptr = the_tasks; sa_ptr != NULL; sa_ptr = sa_tmp) {
		time_t current_time = time(NULL);
		if(current_time>=sa_ptr->nextTrigger){
			
			cron_task_t *s = (cron_task_t *)param;
			(*((s)->thecallback)) ((s)->clientreg, (s)->clientarg);
		
			globalUpdateNextTrigger();
			the_tasks=toSortList(the_tasks,s);
			next_runtime=the_tasks->nextTrigger;
		}
		sa_tmp = sa_ptr->next;
	}
}

struct cron_task* toSortList(struct cron_task* head, struct cron_task* tail) {
	if (head == NULL) {
		return head;
	}
	if (head->next == tail) {
		head->next = NULL;
		return head;
	}
	struct cron_task *slow = head, *fast = head;
	while (fast != tail) {
		slow = slow->next;
		fast = fast->next;
		if (fast != tail) {
			fast = fast->next;
		}
	}
	struct cron_task* mid = slow;
	return merge(toSortList(head, mid), toSortList(mid, tail));
}
struct cron_task* merge(struct cron_task* head1, struct cron_task* head2) {
    struct cron_task* dummyHead = malloc(sizeof(struct cron_task));
    dummyHead->nextTrigger = 0;
    struct cron_task *temp = dummyHead, *temp1 = head1, *temp2 = head2;
    while (temp1 != NULL && temp2 != NULL) {
        if (temp1->nextTrigger <= temp2->nextTrigger) {
            temp->next = temp1;
            temp1 = temp1->next;
        } else {
            temp->next = temp2;
            temp2 = temp2->next;
        }
        temp = temp->next;
    }
    if (temp1 != NULL) {
        temp->next = temp1;
    } else if (temp2 != NULL) {
        temp->next = temp2;
    }
    return dummyHead->next;
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




