#include "common.cpp"
#include <unistd.h>
#include <sys/un.h>
#include <pthread.h>
// For sockaddr_in
#include <netinet/in.h>
// For port - htons
#include <arpa/inet.h>

#define BUF_SIZE 100
using namespace std; 


struct thread_args {
  int sfd;
};

void *handle_connection(void *t);
int writeToUdpClient(const char* buf , int fd , sockaddr *clientaddr,socklen_t clientlen) {
  int n = sendto(fd,buf, strlen(buf), 0, clientaddr, clientlen);
  return n;
};
// Creating server 
int create_server(int port) {
  int sfd = socket(AF_INET , SOCK_DGRAM , 0);
  if (sfd == -1)                
    error("socket err");
  // struct sockaddr_in *my_addr, *peer_addr;
  /* Initialize socket structure */
  struct sockaddr_in serv_addr;  
  struct sockaddr_in clientaddr; 
  char buf[1255]; /* message buf */
  int newsockfd;

  bzero((char *) &serv_addr, sizeof(serv_addr));   
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(port);

  if(bind (sfd , (struct sockaddr*) &serv_addr , sizeof(serv_addr)) < 0)
    error("Unable to bind \n");
  else 
    cout<<"Binding complete to "<<port<<" \n";

  while(1) {
    pthread_t thread;
    thread_args *args = new thread_args();
    bzero((char *) args, sizeof(thread_args));

    socklen_t clientlen = (socklen_t)sizeof(clientaddr);

    cout<<"Waiting "<<endl;    
    
    int n = recvfrom(sfd, buf, 1000, 0,
		 (struct sockaddr *) &clientaddr, &clientlen);
    if (n < 0) {
      error("ERROR reading from socket");    
    } else {      
      cout<<"Recieved message \n"<<buf<<endl;    
      // Parse the buffer to get http headers 
      struct http_headers *headers = parseHttpHeaders(buf);
      // cout<<"Here is the message: \n"<<str;
      const char* path = headers->path;
      cout<<endl<<"The Path " << path<<endl;
      cout.flush();
      if (path && strcmp(path,"/") != 0) {
        char respath[] = "www/";
        strcat(respath,path);      
        int m = getFile(respath , writeToUdpClient , sfd,
                        (struct sockaddr *) &clientaddr, clientlen);
        const char* msg = "HTTP/1.0 404 Not Found\n Content-type: text/html \n\n <html><body><h2>Not found </h2></body></html>";
        if (m < 0){
          n = sendto(sfd,msg, strlen(msg), 0, 
                     (struct sockaddr *) &clientaddr, clientlen);
        }
      } else if (!path || strcmp(path,"/") == 0) {
        const char* msg2 = "Hi , Type a path to a file in the www folder if you know one :)";
        /* Write a response to the client */
        n = sendto(sfd,msg2, strlen(msg2), 0, 
                   (struct sockaddr *) &clientaddr, clientlen);
      } else {
        const char* msg3 = "HTTP/1.0 404 Not Found\n Content-type: text/html \n\n <html><body><h2>Not found </h2></body></html>";
        n = sendto(sfd,msg3, strlen(msg3), 0, 
                   (struct sockaddr *) &clientaddr, clientlen);
      }
         
      if (n < 0) {
        error("ERROR writing to socket");
      } else {
        // Send a close message to the waiting client 
        n = sendto(sfd,"", 0, 0, 
                   (struct sockaddr *) &clientaddr, clientlen);
      }
    }
  };  
  return 0;
};



// Creating client
int main (int argc ,char** argv) {  
 int c,port = 2222,tmp ;
 while ((c = getopt (argc, argv, "p:")) != -1) {
   switch(c) {
   case 'p' : 
     tmp = atoi(optarg);
     if(tmp != 0) {
       port = tmp;
       cout<<"Binding port to "<<port << endl;
     }
     break;
   case '?':
     if (optopt == 'p')
       cout<<"Enter port info";
     break;
   default:
     cout<<"Option is "<<c <<endl;
   };
      
 };
 create_server(port);
 return 0;
}
