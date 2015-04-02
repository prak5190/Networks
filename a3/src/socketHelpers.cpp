#include "common.cpp"
#ifndef __SOCKETHELPER__
#define __SOCKETHELPER__ 1 
// Socket helpers !!!!!!!!!!!!
int initSocket() {
  // Create socket 
  int s;			   /* s = socket */
  s = socket(AF_INET, SOCK_DGRAM, 0);
  //  Enable reuse
  int true1 = 1;
  if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *) &true1,
                 sizeof(true1)) == -1) {
    perror("reuseaddr");
    return -1;
  }
  // if (setsockopt(s,IPPROTO_IP,IP_RECVTTL , (char *) &true1,
  //                sizeof(true1)) == -1)
  //   { perror("RecvTTL error");
  //     return -1;
  //   }
  return s;
};

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
  if (log(3.9))
    std::cout << "Got a header " << header->seq << ": " << (int) header->ack << std::endl;
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
