#include "common.cpp"

int sendPacket(sockaddr* addr, socklen_t size , char* packet) {
  int socket = app.socket;
  return sendto(socket,packet , PACKET_SIZE, 0, addr ,size);
};
// Send header
int sendHeader (std::unique_ptr<udp_header> header , sockaddr* addr, socklen_t size) {  
  char packet[PACKET_SIZE];
  writeToBuffer(packet, header.get() , "");
  // Send the ack
  return sendPacket(addr , size , packet);  
};

int sendFileRequest (string fname , sockaddr* addr, socklen_t size) {  
  char packet[PACKET_SIZE];
  fileInfo info;
  info.filename = fname;
  udp_header header;  
  header.isRequest = true;
  createRequestPacket(packet , &header , &info);
  // Send the ack
  //sendto(app.socket, packet , 1500, 0, (const sockaddr*) &forw->clientaddr ,sizeof(sockaddr_in))
  return sendto(app.socket,packet , PACKET_SIZE, 0, addr ,size);
}

// A recieved which parses this data 
void* createReciever (void* args) {
  int port = app.recieve_port;
  int s = app.socket;
  // Bind listner socket
  if (bindUdpSocket(port,s) != -1) {
    printf("OK: bind SUCCESS\n");
  } else {
    printf("Error: bind FAILED\n");
  }
  
  char buffer[PACKET_SIZE];
  std::unique_ptr<char>data(new char[1500]);
  struct sockaddr_in recv_addr;   udp_header header;
  socklen_t recv_len = (socklen_t)sizeof(recv_addr);
  int count = 0;
  //msghdr *msg = new msghdr();
  // Boiler plate to get ttl
  struct msghdr msg; 
  int *ttlptr=NULL;
  int received_ttl = 0;
  char buf[CMSG_SPACE(sizeof(received_ttl))];
  msg.msg_control = buf; // Assign buffer space for control header + header data/value
  msg.msg_controllen = sizeof(buf); //just initializing it
  //bzero(msg, sizeof(msghdr));
  while((recvmsg(s,&msg, MSG_PEEK) >= 0)
        &&
        (recvfrom(s,buffer,PACKET_SIZE,0,
                  (struct sockaddr *) &recv_addr , &recv_len) >= 0)) {
    // Get TTL 
    {
      struct cmsghdr *cmsg;
      for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL; cmsg = CMSG_NXTHDR(&msg,cmsg)) {
        if ((cmsg->cmsg_level == IPPROTO_IP) && (cmsg->cmsg_type == IP_TTL) &&
            (cmsg->cmsg_len) ){
          ttlptr = (int *) CMSG_DATA(cmsg);
          received_ttl = *ttlptr;
          std::cout << "Recieved ttl " << received_ttl << std::endl;
          break;
        }
      }
    }

    readPacket(buffer,&header,data.get());
    windowCounter--;
    if (header.ack > 0) {
      // Then this is an ack , send it to the sender 
      if (windowCounter > 10) {
        thread_resume();
      }
    } else {
      // This is new data , handle it accordingly 
      // Send out an ack
      if (data.get() != NULL) {
        // Lets write to a temp file 
        //std::cout << data.get() << std::endl;
      }
      std::unique_ptr<udp_header>k(new udp_header());
      k->ack = count++;
      sendHeader(std::move(k),(struct sockaddr *) &recv_addr ,recv_len);
    }
    // Blank the buffer
    bzero(buffer, sizeof(buffer));
    //bzero(msg, sizeof(msghdr));
  }
  return 0;
}

struct CbArgs {
  sockaddr_in clientaddr;
};

int processBuffer(std::unique_ptr<func_args> args) {
  CbArgs *forw = (CbArgs*)args->forw;
  File_stats *fs = (File_stats*) args->func;
  udp_header k;
  // No ack present , represented by setting it to 0
  k.ack = 0;
  k.ttl = 9901;  
  char packet[1500];
  long seqnum = fs->seq;
  if(fs->eof){
    std::cout << "FIle Complete " << std::endl;
    return 0;
  }

  writeToBuffer(packet, &k , fs->data);
  int n = sendto(app.socket, packet , 1500, 0, (const sockaddr*) &forw->clientaddr ,sizeof(sockaddr_in));   
  std::cout << "Sequence Num " << seqnum << std::endl;
  // Wait if 10 packets sent
  if (windowCounter > 10) {    
    timespec time;
    thread_timed_wait(2);
  };
  windowCounter++;  
  return 0;
}

int sendToClient(sockaddr_in clientaddr,const char *respath , long start , long offset ,int (*cb) (std::unique_ptr<func_args>)) {
  int sfd = app.socket;  
  socklen_t clientlen = sizeof(clientaddr);  
  CbArgs *cbArguments = new CbArgs();  
  cbArguments->clientaddr = clientaddr;
  int m = getFile(respath , PACKET_SIZE - sizeof(udp_header) ,start, offset , cb , cbArguments);
  if (m < 0) {
    std::cout << "Failed" << std::endl;
  }
}


// Threaded apps
void* getDataInThread(void* args) {
  thread_args *somargs = (thread_args*)args;
  int port = somargs->port;
  string ip = somargs->ip;
  string fname = somargs->filename; 
  sockaddr_in cl_addr = getSocketAddr(ip,port);
  
  // 
  sendToClient(cl_addr , "../www/file" , 0 , -1 , processBuffer); 
  return NULL;
}
