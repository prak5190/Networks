#include "common.cpp"
#ifndef __SOCKETHELPER__
#define __SOCKETHELPER__ 1 
// Socket helpers !!!!!!!!!!!!
int bindUdpSocket(int port , int s) {
  sockaddr_in in_addr;	   /* Structure used for bind() */
  in_addr.sin_family = AF_INET;                   /* Protocol domain */
  in_addr.sin_addr.s_addr = 0;                    /* Use wildcard IP address */  
  in_addr.sin_port = port;	       	   /* Use this UDP port */
  return bind(s, (struct sockaddr *)&in_addr, sizeof(in_addr));
}

sockaddr_in getSocketAddr(string ip , int port) {
  struct hostent *server;
  server = gethostbyname(ip.c_str());
  int socket = app.socket;
  sockaddr_in serv_addr;
  bzero((char *) &serv_addr, sizeof(sockaddr_in));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = port;
  return serv_addr;
}

// Reads data into udp_header 
void readPacket(char* buffer , udp_header *header , char* data) {
  if (buffer == NULL)
    return;
  size_t sz = sizeof(udp_header);
  memcpy(header , buffer , sz);
  memcpy(data , &buffer[sz]  , PACKET_SIZE - sz);
};
void readPacket(char* buffer , udp_header *header , fileInfo *data) {
  if (buffer == NULL)
    return;
  size_t sz = sizeof(udp_header);
  memcpy(header , buffer , sz);
  memcpy(data , &buffer[sz]  , sizeof(fileInfo));
};


void getFileInfoFromData(char* data, fileInfo *f) {
  memcpy(f, data , sizeof(fileInfo));  
}
void createRequestPacket(char* buffer, udp_header *header, fileInfo *finfo) {
  size_t sz = sizeof(udp_header);
  memcpy(buffer, header, sz);
  memcpy(&buffer[sz] , finfo , sizeof(fileInfo));
};

void writeToBuffer(char* buffer, udp_header *header, const char* data) {
  size_t sz = sizeof(udp_header);
  memcpy(buffer, header, sz);
  memcpy(&buffer[sz] , data , PACKET_SIZE - sz);
};
#endif
