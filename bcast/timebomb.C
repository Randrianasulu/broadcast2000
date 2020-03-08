#include <string.h>
#include "timebomb.h"
#include <stdio.h>
#include <sys/stat.h>


#define LASTYEAR 1999
#define LASTDAY 1
#define LASTMONTH 12
#define EXCUSE \
"We're determined to give it out for free but so far the unemployment \
rate is higher than the humidity.  Since a bank, not a day job, loaned \
all the money for the software it is impossible to release it until we \
can pay for it.  In the mean time, buy American, get straight A's, and \
there might be a fee-based CD distribution for just those who can't \
live without Broadcast 2000.\n"



TimeBomb::TimeBomb()
{
	struct stat fileinfo;
	time_t system_time;
	int result;

	result = stat("/etc", &fileinfo);
	system_time = time(0);

	printf("Demo expires %d/%d/%d\n", LASTMONTH, LASTDAY, LASTYEAR);

	if(test_time(fileinfo.st_mtime) ||
		test_time(system_time))
	{
		printf(
"*** This Broadcast 2000 demo has expired\n"
EXCUSE
		);
//		exit(1);
	}
}


int TimeBomb::test_time(time_t testtime)
{
	struct tm *currenttime;
	currenttime = localtime(&testtime);

	if(currenttime->tm_year >= LASTYEAR - 1900 &&
		currenttime->tm_mday >= LASTDAY &&
		currenttime->tm_mon >= LASTMONTH - 1) return 1;
	else return 0;
return 0;
}

TimeBomb::~TimeBomb()
{
}
