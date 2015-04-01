#include "common.cpp"
void timespec_now(struct timespec *ts)
{
  struct timeval  tv;
  // get the current time
  gettimeofday(&tv, NULL);
  ts->tv_sec=tv.tv_sec;
  ts->tv_nsec=tv.tv_usec*1000;  
}

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

/****** Jacob orams algo ***********************/
// All calculations in millis
long calculateRTT(long sampleRtt , long estRtt , long deviation) {
  if (sampleRtt == 0)
    return estRtt;
  if(log(4))
    std::cout << "Sample "<<sampleRtt << " estRtt " <<estRtt << " Dev " << deviation << std::endl;
  // 3 is bacase of sigma = 1/8 - i.e unshift by 3
  sampleRtt -= (estRtt >> 3) ;
  estRtt += sampleRtt; 
  if (sampleRtt < 0)
    sampleRtt = -sampleRtt;
  sampleRtt -= (deviation >> 3);
  deviation += sampleRtt;
  // Timout
  return (estRtt >> 3) + (deviation >> 1);
}
