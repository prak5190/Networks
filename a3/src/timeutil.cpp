#include "common.cpp"
void timespec_addms(struct timespec *ts, long ms)
{
  int sec=ms/1000;
  ms=ms-sec*1000;
  // perform the addition
  ts->tv_nsec+=ms*1000000;
  // adjust the time
  ts->tv_sec+=ts->tv_nsec/1000000000 + sec;
  ts->tv_nsec=ts->tv_nsec%1000000000;
}

int timespec_compare(struct timespec *a, struct timespec *b)
{
	if (a->tv_sec!=b->tv_sec)
		return a->tv_sec-b->tv_sec;

	return a->tv_nsec-b->tv_nsec;
}

// computes a = a-b
void timespec_subtract(struct timespec *a, struct timespec *b)
{
	a->tv_nsec = a->tv_nsec - b->tv_nsec;
	if (a->tv_nsec < 0) {
		// borrow.
		a->tv_nsec += 1000000000;
		a->tv_sec --;
	}

	a->tv_sec = a->tv_sec - b->tv_sec;
}

// convert the timespec into milliseconds (may overflow)
int timespec_milliseconds(struct timespec *a) 
{
	return a->tv_sec*1000 + a->tv_nsec/1000000;
}
