#include "common.cpp"
#ifndef __STATE_CPP__
#define __STATE_CPP__ 1 
void printSenderStatistics();
struct appglobals {
  int socket;
  // This mutex tells the sender to wait when window is filled
  pthread_mutex_t mut ; 
  pthread_cond_t  cond ;
  long max_window_size;
  int recieve_port;
  bool isDupElseSel;
  bool hasLatency, hasDrops;
  long latencyTime;
  float drop_probability;
} app;
struct SenderState {
  long windowSize, seqNum , expSeqNum = 999999999 , lastAckedNum, windowCounter;
  timespec  *lastMsgTime; 
  timespec startTime;
  // Rtt in milliseconds
  long rtt = 0;
  bool isSending ,isAimdorSlow = false;
  // Address of last reciever 
  sockaddr_in  recv_addr;
  socklen_t recv_len;
  pthread_mutex_t mut     = PTHREAD_MUTEX_INITIALIZER;
  pthread_cond_t  cond   = PTHREAD_COND_INITIALIZER;
  void wait() {
    thread_wait(this->mut , this->cond);
  }
  void resume() {
    thread_resume(this->mut, this->cond);
  }
  long slowStartTransferredSize,fileSize;
  void printFinalStats() {
    double slowPercent =(double) (this->slowStartTransferredSize * 100) /this->fileSize ;
    std::cout << this->slowStartTransferredSize << std::endl;
    std::cout << this->fileSize << std::endl;    
    std::cout << "Percentage transferred in slow Start " << slowPercent  << std::endl;
    std::cout << "Percentage transferred in AIMD " << 100-slowPercent << std::endl;
    timespec curr;
    timespec_now(&curr);
    timespec_subtract(&curr, &(this->startTime));
    long timeTaken =  timespec_milliseconds(&curr);
    std::cout << "Time take in milliseconds " <<timeTaken << std::endl;    
  }
  bool isCongested;
  long flipWindowSize;
  void handleWindow(bool increase) {
    if (increase) {
      if (this->isAimdorSlow) {
        if (this->windowSize < app.max_window_size)
          this->windowSize += 1;
      } else {
        if (this->isCongested) {
          if (this->windowSize * 2 < this->flipWindowSize) {
            this->windowSize += this->windowSize;
          } else {
            this->slowStartTransferredSize = (this->lastAckedNum + 1) * DATA_SIZE;
            this->isAimdorSlow = true;            
            this->windowSize+= 1;
          }            
        } else
          this->windowSize += this->windowSize;
      }
      
      if (this->windowSize > app.max_window_size) 
        this->windowSize = app.max_window_size;
    } else {
      this->handleDrop();
    }
  }
  void handleDrop() {    
    //this->isAimdorSlow = true;    
    if (log(4.5))
      std::cout << "Packet Dropped" << std::endl;    
    this->windowCounter = 0;
    if (this->isAimdorSlow) {
      this->windowSize = this->windowSize / 2;
    } else {
      this->isCongested = true;
      this->windowSize = 1;
    }
    printSenderStatistics();
    this->resume();
  }
  int waitTime(long ms) {
    // RTT in ms , so lets take some margin
    ms  += 10;
    return thread_timed_wait(this->mut,this->cond,ms);
  }
  void setTime() {
    timespec* lastMsgTime = this->lastMsgTime;
    if (lastMsgTime == NULL) 
      lastMsgTime = new timespec();
    timespec_now(lastMsgTime);
    this->lastMsgTime = lastMsgTime;
  }
  void setRTT() {
    // Get prev time 
    timespec *prev = this->lastMsgTime;
    // Get current time 
    this->lastMsgTime = new timespec();    
    this->setTime();    
    long sampleRtt;
    if(prev != NULL) {
      timespec_subtract(this->lastMsgTime,prev);
      sampleRtt = timespec_milliseconds(this->lastMsgTime);
      delete prev;
    } else {
      this->setTime();
      return;
    }
    long estRtt = this->rtt;
    long deviation = sampleRtt - estRtt;
    if (estRtt == 0) {
      // Just set it to sample
      this->rtt = sampleRtt;
    } else {
      // Calcualate RTT 
      this->rtt =  calculateRTT(sampleRtt , estRtt , deviation);
    }
    this->setTime();
  }
  ~SenderState () {
    delete this->lastMsgTime;
  }
} senderState;
struct RecieverState {
  bool isRecieving;
  long windowSize, lastRecievedSeq , seqBase , seqMax,windowCounter , fileSize;
  timespec  *lastMsgTime; 
  long initialFileSize;
  string filename;
  // Rtt in milliseconds
  long rtt = 0;
  long totalPackets;
  bool fileClosed;
  // Address of last reciever 
  sockaddr_in  recv_addr;
  socklen_t recv_len;
  pthread_mutex_t mut    = PTHREAD_MUTEX_INITIALIZER;
  pthread_cond_t  cond   = PTHREAD_COND_INITIALIZER;
  void wait() {
    thread_wait(this->mut , this->cond);
  }
  void resume() {
    thread_resume(this->mut, this->cond);
  }
  int waitTime(int ms) {
    return thread_timed_wait(this->mut,this->cond,ms);
  }
  void setTime() {
    timespec* lastMsgTime = this->lastMsgTime;
    if (lastMsgTime == NULL) 
      lastMsgTime = new timespec();
    timespec_now(lastMsgTime);
    this->lastMsgTime = lastMsgTime;
  }
  void setRTT() {
    // Get prev time 
    timespec *prev = this->lastMsgTime;
    // Get current time 
    this->lastMsgTime = new timespec();    
    this->setTime();    
    long sampleRtt;
    if(prev != NULL) {
      timespec_subtract(this->lastMsgTime,prev);
      sampleRtt = timespec_milliseconds(this->lastMsgTime);
      delete prev;
    } else {
      this->setTime();
      return;
    }
    long estRtt = this->rtt;
    long deviation = sampleRtt - estRtt;
    if (estRtt == 0) {
      // Just set it to sample
      this->rtt = sampleRtt;
    } else {
      // Calcualate RTT 
      this->rtt =  calculateRTT(sampleRtt , estRtt , deviation);
    }
    this->setTime();
  }
  void handleDrop() {
  }

  ~RecieverState () {
    delete this->lastMsgTime;
  }
} recieverState;

// Print Utils - 
float lastProgress;
void printRecProgress() {
  float progress = ((recieverState.initialFileSize - recieverState.fileSize)) * 100/ (recieverState.initialFileSize); 
  //std::cout << "Progress " << progress << std::endl;
  if (progress > lastProgress + 0.5) {
    if (((int)progress) % 10 == 0) {
      std::printf("Progress %.0f \n", progress);
      std::cout << "RTT " << recieverState.rtt  << std::endl;
    }
    lastProgress = progress;
  }
  //   std::printf("Progress %.2f \r", progress);
  // }
}

long oldWindowSize;
void printSenderStatistics() {
  if (senderState.windowSize != oldWindowSize) {    
    if (senderState.windowSize - oldWindowSize == 1)
      std::cout << "In Additive increase : ";
    else if (oldWindowSize!= 0 &&   senderState.windowSize/oldWindowSize == 2) 
      std::cout << "In Slow Start : ";
    else if (senderState.windowSize!= 0 &&   oldWindowSize/senderState.windowSize == 2) 
      std::cout << "In Multiplicative decrease : ";
    std::cout << "Max Window Size " << senderState.windowSize << " : Window Counter  " << senderState.windowCounter << std::endl;
    oldWindowSize = senderState.windowSize;
  }
}

#endif

