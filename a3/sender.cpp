/* File that defines sending of files*/
#include "common.cpp"
using namespace std;
// Whatever items are required are set in global vars 
int w_socket;
sockaddr *w_addr;
int writeToUdpClient(const char* buf) {  
  //cout<<"Data reciefved "<<buf << endl;  
  udp_header k;
  k.ttl = 9901;
  char  packet[1500];
  memcpy(packet, &k , sizeof(udp_header));
  memcpy(&packet[sizeof(udp_header)] , buf , 1500 - sizeof(udp_header));
  int n = sendto(w_socket,packet , 1500, 0, w_addr,sizeof(sockaddr_in)); 
  return n;
};

int sendToClient(sockaddr_in clientaddr) {
  int sfd = socket(AF_INET , SOCK_DGRAM , 0);
  // struct sockaddr_in *my_addr, *peer_addr;
  /* Initialize socket structure */
  char respath[] = "www/file";
  //strcat(respath,path);
  socklen_t clientlen = sizeof(clientaddr);
  w_socket = sfd;
  w_addr = (struct sockaddr*) &clientaddr;
  int m = getFile(respath , writeToUdpClient , 1500 - sizeof(udp_header));
  if (m < 0) {
    cout<<"failed ";
  }
}

// int main (int argc , char** argv) {
//   struct sockaddr_in serv_addr;
//   struct hostent *server;
//   server = gethostbyname("localhost");
//   bzero((char *) &serv_addr, sizeof(serv_addr));  
//   serv_addr.sin_family = AF_INET;
//   bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
//   serv_addr.sin_port = htons(9000);

//   //s = socket(AF_INET, SOCK_DGRAM, 0);
//   sendToClient(serv_addr);
//   return 0;
// }
