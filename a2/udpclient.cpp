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
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  
  if (sockfd < 0) {
      perror("ERROR opening socket");
      exit(1);
  };
  
  server = gethostbyname(name);

  portno = 2222;
  bzero((char *) &serv_addr, sizeof(serv_addr));  
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(portno);

  /* Now connect to the server */
  if (connect(sockfd,(struct sockaddr *) &serv_addr ,sizeof(serv_addr)) < 0) {
    error("ERROR connecting");
  } else {    
    while(1) {
      char buf[1000],buffer[1000];
      bzero(buffer, sizeof(buffer));  
      cout<<" Enter message to send to server "<<endl;      
      cin>>buf;
      /* Send message to the server */
      socklen_t clientlen = (socklen_t)sizeof(serv_addr);
      cout<<"sending message "<<buffer;
      n = sendto(sockfd, buf, strlen(buf), 0, 
                 (struct sockaddr *) &serv_addr, clientlen);
      if (n < 0)    {
        error("ERROR writing to socket");
      } else {
        // Read server response and print out
        int n = recvfrom(sockfd,buf,10000,0,NULL,NULL);
        cout<<"\n Response : \n" << buf;        
      }
    };
  }
       
  if (n < 0)    {
    error("ERROR reading from socket");
  }

  printf("%s\n",buffer);
  close(sockfd); 
  return 0;
}

// Creating client
int main (int argc ,char** argv) {  
  client();
  return 0;
}
