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

// Creating client
int client (int portno , const char* host , const char* path ) {
  // Start time
  clock_t start, end;
  double cpu_time_used;
  start = clock();

  //create_server
  int sockfd, n;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  
  if (sockfd < 0) {
      perror("ERROR opening socket");
      exit(1);
  };
  
  server = gethostbyname(host);

  bzero((char *) &serv_addr, sizeof(serv_addr));  
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(portno);

  char buf[1000],buffer[1000];
  bzero(buffer, sizeof(buffer));  
  /* Send message to the server */
  char message[1000];
  bzero(message , sizeof(message));
  char protocol[] = "1.0";
  sprintf(message,HTTP_HEADER ,path, protocol , host);

  socklen_t clientlen = (socklen_t)sizeof(serv_addr);
  cout<<"sending message "<<buffer;
  long totalSize = 0;

  n = sendto(sockfd, message, strlen(message), 0, 
             (struct sockaddr *) &serv_addr, clientlen);
  if (n < 0)    {
    error("ERROR writing to socket");
  } else {
    while(1){
      // Read server response and print out
      int n = recvfrom(sockfd,buf,1000,0,NULL,NULL);
      totalSize += n;
      if (n == 0)
        break;
    };
  }
  cout<<endl<<"Connection closed , \n Total Size is : "<<totalSize;
  end = clock();
  cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
  cout<<" Time taken "<<cpu_time_used<<endl;
  cout.flush();
  if (n < 0)    {
    error("ERROR reading from socket");
  }

  close(sockfd); 
  return 0;
}

// Creating client
int main (int argc ,char** argv) {  
  signal(SIGSEGV, unknownexit);
  if(argc > 1) {
    client_args args = getClientArgs(argc , argv);
    client(args.port , args.host , args.path);
  } else {
    cout<<"Please enter port info -p , host info -h and file info -f";
  };
  return 0;
}
