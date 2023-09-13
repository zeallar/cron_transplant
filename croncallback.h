
typedef void    (CronCallback) (unsigned int clientreg,
                                         void *clientarg);
#define CRON_MALLOC_STRUCT(s)   (struct s *) calloc(1, sizeof(struct s))
typedef struct cron_task {
        char* schedule;
        unsigned int    clientreg;
        void           *clientarg;
        CronCallback *thecallback;
        struct cron_task *next;
    }cron_task_t;

unsigned int 
cron_callback_register(char* when,
                    CronCallback * thecallback, void *clientarg);
void cron_callback_unregister(unsigned int clientreg);
int nap(cron_task_t *job);
void *run_job(void *param);
void cron_callback_unregister_all(void);