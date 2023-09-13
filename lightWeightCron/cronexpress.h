#ifndef CRONEXPRESS_H
#define CRONEXPRESS_H

#define FIELD_MINUTES   60
#define FIELD_HOURS     24
#define FIELD_M_DAYS    32
#define FIELD_MONTHS    12
#define FIELD_W_DAYS     7

#define FIRST_DOW  (1 << 0)
#define SECOND_DOW (1 << 1)
#define THIRD_DOW  (1 << 2)
#define FOURTH_DOW (1 << 3)
#define FIFTH_DOW  (1 << 4)
#define LAST_DOW   (1 << 5)
#define ALL_DOW    (FIRST_DOW|SECOND_DOW|THIRD_DOW|FOURTH_DOW|FIFTH_DOW|LAST_DOW)
typedef struct {
    char	cl_Mins[FIELD_MINUTES];	/* 0-59				*/
    char	cl_Hrs[FIELD_HOURS];	/* 0-23					*/
    char	cl_Days[FIELD_M_DAYS];	/* 1-31					*/
    char	cl_Mons[FIELD_MONTHS];	/* 0-11				*/
    char	cl_Dow[FIELD_W_DAYS];	/* 0-6, beginning sunday		*/
} cron_expr;
void cron_parse_expr(const char* expression, cron_expr* target, const char** error);

#endif