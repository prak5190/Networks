#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include "common.cpp"
#include <sys/types.h>          
#include <sys/socket.h>
#include <sys/un.h>
// For sockaddr_in
#include <netinet/in.h>
// For port - htons
#include <arpa/inet.h>
// For gethostbyname
#include <netdb.h>

#define BUF_SIZE 100
using namespace std; 

void ondata (const char* data , int fd) {
  cout<<data;
  if(strlen(data) == 0){
    close(fd);
  };
};

// Creating client
int client (int portno , const char* host , const char* path ,int protocol) {
  //create_server
  int sockfd, n;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  
  char buffer[256];
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  
  if (sockfd < 0) {
      perror("ERROR opening socket");
      exit(1);
  };
  
  server = gethostbyname(host);

  bzero((char *) &serv_addr, sizeof(serv_addr));  
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(portno);

  /* Now connect to the server */
  if (connect(sockfd,(struct sockaddr *) &serv_addr ,sizeof(serv_addr)) < 0) {
    error("ERROR connecting");
  }
  
  char message[1000];
  bzero(message , sizeof(message));
  char proto[] = "1.%d";
  char pro[10];
  sprintf(pro,proto,protocol);
  sprintf(message,HTTP_HEADER ,path, pro , host);
  cout<<"Message sent to server " <<message;
  /* Send message to the server */
  n = send(sockfd,message,strlen(message),0);
   
  if (n < 0)    {
    error("ERROR writing to socket");
  }
   
  /* Now read server response until length*/
  readFromSocket(sockfd,n,ondata);
  // bzero(buffer,256);
  // n = read(sockfd,buffer,255);   
  if (n < 0)    {
    error("ERROR reading from socket");
  }

  return 0;
};

void shutdown(int dummy=0) {
    cout<<endl<<"Shutting down server";
    cout.flush();
    exit(1);
};


// Creating client
int main (int argc ,char** argv) {  
  signal(SIGSEGV, unknownexit);
  if(argc > 1) {
    client_args args = getClientArgs(argc , argv);
    signal(SIGINT, shutdown);
    client(args.port , args.host , args.path,args.protocol);
  } else {
    cout<<"Please enter port info -p , host info -h and file info -f and protocol -r 0 for persistent and 1 for non persistent ";
  }
  return 0;
}
