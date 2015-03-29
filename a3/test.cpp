#include "common.cpp"
#include <iostream>
#include <inttypes.h>
using namespace std;
int print(char* buf) {
  cout<<buf<< endl;
  return 0;
}

void testSerialization(){
  udp_header k;
  k.ttl = 100;
  const char* buf = "Hello World";
  char  packet[1500];
  memcpy(packet, &k , sizeof(udp_header));
  memcpy(&packet[sizeof(udp_header)] , buf , 1500 - sizeof(udp_header));
  
  // Now deserialize the file
  udp_header header;
  char nbuf[1000];
  memcpy(&header, packet , sizeof(udp_header));
  memcpy(nbuf , &packet[sizeof(udp_header)]  , 1500 - sizeof(udp_header));

  std::cout << nbuf << std::endl;
};

struct thread_args {
  int port;
};

pthread_mutex_t mut   = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  cond   = PTHREAD_COND_INITIALIZER;
// This thread waits
void* thread1(void* args) {
  cout<<"Entered t1"<<endl;
  thread_args *t = (thread_args*)args;
  int port = t->port;

  // while (x <= y) {
  //   pthread_cond_wait(&cond, &mut);
  // }
  for (int i=0;i<10;i++) {
    pthread_mutex_lock(&mut);
    std::cout << "T1 " <<i << std::endl;
    pthread_cond_broadcast(&cond);
    pthread_cond_wait(&cond, &mut);
    pthread_mutex_unlock(&mut);
  }
  /* operate on x and y */
  // pthread_mutex_unlock(&mut);
  return NULL;
}
// This thread changes the wait condition
void* thread2(void* args) {
  std::cout << "Enter t2" << std::endl;
  for (int i=0;i<10;i++) {
    pthread_mutex_lock(&mut);
    std::cout << "T2 " <<i << std::endl;
    pthread_cond_broadcast(&cond);
    pthread_cond_wait(&cond, &mut);
    pthread_mutex_unlock(&mut); 
  }
  return NULL;
}
int thr1 , thr2 ;
void testThreads(){
  pthread_t t1 , t2;
  thread_args *args = new thread_args();
  args->port = 1000;
  if ((thr1 = pthread_create(&t1 , NULL ,thread1, args)) == 0) {
    std::cout << "Thread1 created" << std::endl;
  }
  if ((thr2 = pthread_create(&t2 , NULL ,thread2, args)) == 0) {
    std::cout << "Thread2 created" << std::endl;
  }
  pthread_join(t1,NULL);
  pthread_join(t2,NULL);
}

int testMemcpy (int ccot, char ** asd) {
  udp_header kk;
  kk.ttl = 1222;
  char *mem = new char[sizeof(udp_header)];
  memcpy(mem, &kk , sizeof(kk));
  udp_header ll ;  
  memcpy(&ll, mem , sizeof(kk));
  cout<<"TTL Value is " <<ll.ttl;
  return 0;
}


int main(int argus , char **argv){
  //testSerialization();
  //getFile("test.cpp",print,10);
  // testThreads();
  return 0;
}
