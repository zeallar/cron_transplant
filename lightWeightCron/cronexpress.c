#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cronexpress.h"

#define RW_BUFFER		512


#define arysize(ary)	(sizeof(ary)/sizeof((ary)[0]))


#define FIRST_DOW  (1 << 0)
#define SECOND_DOW (1 << 1)
#define THIRD_DOW  (1 << 2)
#define FOURTH_DOW (1 << 3)
#define FIFTH_DOW  (1 << 4)
#define LAST_DOW   (1 << 5)
#define ALL_DOW    (FIRST_DOW|SECOND_DOW|THIRD_DOW|FOURTH_DOW|FIFTH_DOW|LAST_DOW)
const char *MonAry[] = {
	"jan",
	"feb",
	"mar",
	"apr",
	"may",
	"jun",
	"jul",
	"aug",
	"sep",
	"oct",
	"nov",
	"dec",

	"Jan",
	"Feb",
	"Mar",
	"Apr",
	"May",
	"Jun",
	"Jul",
	"Aug",
	"Sep",
	"Oct",
	"Nov",
	"Dec",
	NULL
};
const char *DowAry[] = {
	"sun",
	"mon",
	"tue",
	"wed",
	"thu",
	"fri",
	"sat",

	"Sun",
	"Mon",
	"Tue",
	"Wed",
	"Thu",
	"Fri",
	"Sat",
	NULL
};
char *
ParseField(char *ary, int modvalue, int offset, int onvalue, const char **names, char *ptr);

void
FixDayDow(cron_expr *expr);
void cron_parse_expr(const char* expression, cron_expr* target, const char** error){
    const char* err_local;
    size_t len = 0;
    char buf[RW_BUFFER]; /* max length for cronexpress */
    if (!error) {
        error = &err_local;
    }
    *error = NULL;
    if (!expression) {
        *error = "Invalid NULL expression";
    }
    if (!target) {
        *error = "Invalid NULL target";
    }
    memset(buf,'0',sizeof(buf));
    strcpy(buf,expression);
	
	printf( "Entry %s\n", buf);
	
    char *ptr = buf;
    while (*ptr == ' ')
		++ptr;
    /*
    * parse date ranges
    */
    ptr = ParseField(target->cl_Mins, FIELD_MINUTES, 0, 1,
            NULL, ptr);
    ptr = ParseField(target->cl_Hrs,  FIELD_HOURS, 0, 1,
            NULL, ptr);
    ptr = ParseField(target->cl_Days, FIELD_M_DAYS, 0, 1,
            NULL, ptr);
    ptr = ParseField(target->cl_Mons, FIELD_MONTHS, -1, 1,
            MonAry, ptr);
    ptr = ParseField(target->cl_Dow,  FIELD_W_DAYS, 0, ALL_DOW,
            DowAry, ptr);

    /*
    * fix days and dow - if one is not * and the other
    * is *, the other is set to 0, and vise-versa
    */

	FixDayDow(target);
}
/* Reconcile Days of Month with Days of Week.
 * There are four cases to cover:
 * 1) DoM and DoW are both specified as *; the task may run on any day
 * 2) DoM is * and DoW is specific; the task runs weekly on the specified DoW(s)
 * 3) DoM is specific and DoW is *; the task runs on the specified DoM, regardless
 *    of which day of the week they fall
 * 4) DoM is in the range [1..5] and DoW is specific; the task runs on the Nth
 *    specified DoW. DoM > 5 means the last such DoW in that month
 */
void
FixDayDow(cron_expr *line)
{
	unsigned short i;
	short DowStar = 1;
	short DomStar = 1;
	char mask = 0;

	for (i = 0; i < arysize(line->cl_Dow); ++i) {
		if (line->cl_Dow[i] == 0) {
			/* '*' was NOT specified in the DoW field on this CronLine */
			DowStar = 0;
			break;
		}
	}

	for (i = 0; i < arysize(line->cl_Days); ++i) {
		if (line->cl_Days[i] == 0) {
			/* '*' was NOT specified in the Date field on this CronLine */
			DomStar = 0;
			break;
		}
	}

	/* When cases 1, 2 or 3 there is nothing left to do */
	if (DowStar || DomStar)
		return;

	/* Set individual bits within the DoW mask... */
	for (i = 0; i < arysize(line->cl_Days); ++i) {
		if (line->cl_Days[i]) {
			if (i < 6)
				mask |= 1 << (i - 1);
			else
				mask |= LAST_DOW;
		}
	}

	/* and apply the mask to each DoW element */
	for (i = 0; i < arysize(line->cl_Dow); ++i) {
		if (line->cl_Dow[i])
			line->cl_Dow[i] = mask;
		else
			line->cl_Dow[i] = 0;
	}

	/* case 4 relies on the DoW value to guard the date instead of using the
	 * cl_Days field for this purpose; so we must set each element of cl_Days
	 * to 1 to allow the DoW bitmask test to be made
	 */
	memset(line->cl_Days, 1, sizeof(line->cl_Days));
}
char *
ParseField(char *ary, int modvalue, int offset, int onvalue, const char **names, char *ptr)
{
	char *base = ptr;
	int n1 = -1;
	int n2 = -1;

	if (base == NULL)
		return (NULL);

	while (*ptr != ' ' && *ptr != '\t' && *ptr != '\n') {
		int skip = 0;

		/*
		 * Handle numeric digit or symbol or '*'
		 */

		if (*ptr == '*') {
			n1 = 0;			/* everything will be filled */
			n2 = modvalue - 1;
			skip = 1;
			++ptr;
		} else if (*ptr >= '0' && *ptr <= '9') {
			if (n1 < 0)
				n1 = strtol(ptr, &ptr, 10) + offset;
			else
				n2 = strtol(ptr, &ptr, 10) + offset;
			skip = 1;
		} else if (names) {
			int i;

			for (i = 0; names[i]; ++i) {
				if (strncmp(ptr, names[i], strlen(names[i])) == 0) {
					break;
				}
			}
			if (names[i]) {
				ptr += strlen(names[i]);
				if (n1 < 0)
					n1 = i;
				else
					n2 = i;
				skip = 1;
			}
		}

		/*
		 * handle optional range '-'
		 */

		if (skip == 0) {
			printf("skip failed parsing crontab for user %s: %s\n", "zbl", base);
			return(NULL);
		}
		if (*ptr == '-' && n2 < 0) {
			++ptr;
			continue;
		}

		/*
		 * collapse single-value ranges, handle skipmark, and fill
		 * in the character array appropriately.
		 */

		if (n2 < 0)
			n2 = n1;

		n2 = n2 % modvalue;

		if (*ptr == '/')
			skip = strtol(ptr + 1, &ptr, 10);

		/*
		 * fill array, using a failsafe is the easiest way to prevent
		 * an endless loop
		 */

		{
			int s0 = 1;
			int failsafe = 1024;

			--n1;
			do {
				n1 = (n1 + 1) % modvalue;

				if (--s0 == 0) {
					ary[n1] = onvalue;
					s0 = skip;
				}
			} while (n1 != n2 && --failsafe);

			if (failsafe == 0) {
				printf("failsafe failed parsing crontab for user %s: %s\n", "zbl", base);
				return(NULL);
			}
		}
		if (*ptr != ',')
			break;
		++ptr;
		n1 = -1;
		n2 = -1;
	}

	

	while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n')
		++ptr;


		int i;
		for (i = 0; i < modvalue; ++i)
			if (modvalue == FIELD_W_DAYS)
				printf("%2x ", ary[i]);
			else
				printf("%d", ary[i]);
		printf("\n");

	return(ptr);
}