#include "common.cpp"
#include <iostream>
#include <inttypes.h>
using namespace std;
void testRttCaclulation(){
  senderState.setTime();
  sleep(2);
  senderState.setRTT();
  std::cout << "RTT is " << senderState.rtt << std::endl;
  senderState.setTime();
  sleep(2);
  senderState.setRTT();
  std::cout << "RTT is " << senderState.rtt << std::endl;
  senderState.setTime();
  sleep(2);
  senderState.setRTT();
  std::cout << "RTT is " << senderState.rtt << std::endl;
  senderState.setTime();
  sleep(2);
  senderState.setRTT();
  std::cout << "RTT is " << senderState.rtt << std::endl;
  senderState.setTime();
  sleep(2);
  senderState.setRTT();
  std::cout << "RTT is " << senderState.rtt << std::endl;
}
void testSerialization(){
  udp_header k;
  k.ttl = 100;
  const char* buf = "Hello World";
  char  packet[1500];
  writeToBuffer(packet, &k , buf);
  
  // Now deserialize the file
  udp_header header;
  char nbuf[1500];
  readPacket(packet , &header , nbuf);
  std::cout << nbuf <<strlen(nbuf)<< std::endl;
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


int print(unique_ptr<func_args> buf) {
  //cout<<buf<< endl;
  std::cout << "Printing" << std::endl;
  return 0;
}
bool isTrueAtPos(char a , int pos) {
  // Return true if pos is true
  
}
char setBoolAtPos(char a ,int pos, bool b) {
  return a;
}
void testMap() {
  unordered_map<string, int> kk;
  kk.insert({{"dasdasd",1}});
  kk.insert({{"a",2}});
  auto search = kk.find("a");
  if (search != kk.end()) {
    std::cout << search->second << std::endl;
  }
}

void testFileCopy() {
  char buffer[1500];
  int readpacks;
  long size = getFileSize("../www/a.mp3");  
  // while(getPacketFile("../www/tcpserver",buffer,-1,sizeof(buffer),false) != -1) {
  //   assembleFile("temp",-1 , buffer ,sizeof(buffer), false);    
  //   bzero(&buffer,sizeof(buffer));
  // }
  while (size > 0) {
    long bufSize = sizeof(buffer);
    if (size < bufSize) {
      bufSize = size;
    }
    if (getPacketFile("../www/a.mp3",buffer,-1,bufSize,false) != -1) {
      std::cout << " A " << sizeof(buffer) << std::endl;
      assembleFile("temp2",-1 , buffer ,bufSize, false);    
      bzero(&buffer,sizeof(buffer));
    }
    size -= bufSize;
  }
  
  std::cout << sizeof(buffer) << std::endl;
  closeFile("temp");
}
void testFileInfo() {
  string name = "../www/a.mp3";
  long size = getFileSize(name);
  char buffer[PACKET_SIZE], data[PACKET_SIZE];
  udp_header header , n;
  header.hasFileInfo = true;
  fileInfo finfo;
  fileInfo *nf = new fileInfo();
  finfo.size = getFileSize(name);
  finfo.filename = "Something";
  createRequestPacket(buffer, &header, &finfo);
  readPacket(buffer , &n ,data);
  std::cout << "Has info" << n.hasFileInfo << std::endl;
  //memcpy(nf, data , sizeof(fileInfo));
  getFileInfoFromData(data,nf);
  std::cout << "Finfo " << nf->size << std::endl;
}
int main(int argus , char **argv){
  testFileInfo();
  return 0;
}

  // for(int i = 0 ; i < 1000; i++) {
  //   assembleFile("asd", i , "as",false);
  // }

  //  assembleFile("aq2", 0 ,(char*)"dassd \ne1asddddddddddddd2312 asdasd",true);

  //testRttCaclulation();
  // std::unique_ptr<File_stats> ret(new File_stats());
  // ret->seq = 100;
  //testSerialization();
  // int error ;
  // do {
  //   char a = (int) 0;
  //   std::cout << (int) a  << std::endl;
  //   error = senderState.waitTime(1000);
  //   std::cout << error << ETIMEDOUT<< std::endl;
  // }  while (error == ETIMEDOUT) ;
  // std::cout << "hello" << std::endl;
  
  // if (t.tv_sec == 0) 
  //   std::cout << "nulll" << std::endl;
  //getFile("test.cpp",200, 0 ,-1 ,print, NULL);
  // Test stackoverflow 
  // for(int i=0;i < 10000 ;i++) {
  //   appglobals *k = new appglobals();    
  //   k->recieve_port = 1999;
  //   getFile("test.cpp",200, 0 ,-1 ,print, NULL);
  // }
  //getFile("test.cpp",print,10);
  // testThreads();
  // thread_args *args = new thread_args();
  // args->ip = "loca";
  // //std::cout << args->ip << std::endl;
  // args = new thread_args();
  // std::cout << args->ip << std::endl;
  //getSocketAddr("localhost",9000);
