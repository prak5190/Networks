#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <sys/un.h>
// For sockaddr_in
#include <netinet/in.h>
// For port - htons
#include <arpa/inet.h>

#define BUF_SIZE 100
using namespace std; 

void error (const char* msg) {
  cout<<"Error "<<msg;
  exit(1);
}
// Creating server 
int main (int argc ,char** argv) {
  int sfd = socket(AF_INET , SOCK_STREAM , 0);
  char buffer[256];
  if (sfd == -1)                
    error("socket err");

  // struct sockaddr_in *my_addr, *peer_addr;
   /* Initialize socket structure */
  struct sockaddr_in serv_addr, cli_addr;
  bzero((char *) &serv_addr, sizeof(serv_addr));

   
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(1111);

  // my_addr = new sockaddr_in();
  // my_addr->sin_family = AF_INET; 
  // my_addr->sin_addr = new in_addr();
  // y_addr->sin_addr->s_addr = INADDR_ANY;
  // my_addr->sin_port = htons(11122);

  if(bind (sfd , (struct sockaddr*) &serv_addr , sizeof(serv_addr)) < 0)
    error("Unable to bind");
  else 
    cout<<"Binding complete to 1111";
  while(1){
      listen(sfd,5);
      int newsockfd;
      socklen_t clilen = (socklen_t)sizeof(cli_addr);
      /* Accept actual connection from the client */
      newsockfd = accept(sfd, (struct sockaddr *)&cli_addr, &clilen);

      if (newsockfd < 0) {
        error("ERROR on accept");    
      }
   
      /* If connection is established then start communicating */
      bzero(buffer,256);
      int n = read( newsockfd,buffer,255 );
   
      if (n < 0) {
        error("ERROR reading from socket");    
      }
  
      printf("Here is the message: %s\n",buffer);
  
      /* Write a response to the client */
      n = write(newsockfd,"I got your message",18);
   
      if (n < 0) {
        error("ERROR writing to socket");
      }
       
      shutdown(newsockfd,2);
  };
  
  //  exit(1);
  return 0;
}
