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
int client () {
  //create_server
  int sockfd, portno, n;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  const char* name = "localhost";
  
  char buffer[256];
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  
  if (sockfd < 0) {
      perror("ERROR opening socket");
      exit(1);
  };
  
  server = gethostbyname(name);

  portno = 1111;
  bzero((char *) &serv_addr, sizeof(serv_addr));  
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(portno);

  /* Now connect to the server */
  if (connect(sockfd,(struct sockaddr *) &serv_addr ,sizeof(serv_addr)) < 0) {
    error("ERROR connecting");
  }
  
  bzero(buffer,256);
  strcpy(buffer , "111111");

  /* Send message to the server */
  n = write(sockfd,buffer,strlen(buffer));
   
  if (n < 0)    {
    error("ERROR writing to socket");
  }
   
  /* Now read server response */
  bzero(buffer,256);
  n = read(sockfd,buffer,255);
   
  if (n < 0)    {
    error("ERROR reading from socket");
  }

  printf("%s\n",buffer);
  close(sockfd); 
  return 0;
}
static bool keepRunning = true;

void shutdown(int dummy=0) {
    keepRunning = false;
    cout<<endl<<"Shutting down server";
    cout.flush();
    exit(1);
}

int print(const char* str) {
  cout<<str;
  cout.flush();
  return 0;
}


// Creating client
int main (int argc ,char** argv) {  
  signal(SIGINT, shutdown);
  client();
  return 0;
}
