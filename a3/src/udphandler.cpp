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

int sendFileRequest (const char* fname , sockaddr* addr, socklen_t size) {  
  char packet[PACKET_SIZE];
  fileInfo info;
  strcpy(info.filename,fname);
  udp_header header;  
  header.isRequest = true;
  createRequestPacket(packet , &header , &info);
  // Send the ack
  //sendto(app.socket, packet , PACKET_SIZE, 0, (const sockaddr*) &forw->clientaddr ,sizeof(sockaddr_in))
  return sendto(app.socket,packet , PACKET_SIZE, 0, addr ,size);
}

void handleRecFStat (udp_header header , char* data , sockaddr_in recv_addr , socklen_t recv_len) {
  fileInfo fs;// = new fileInfo();
  std::cout << "Data " << data << std::endl;
  
  memcpy(&fs, data , sizeof(fileInfo));  
  //getFileInfoFromData(data,fs); 
  std::cout << "File to be recieved " << fs.filename << std::endl;
  std::cout << "FS Size " << fs.size << std::endl;
  sleep(5);

  
  //recieverState.fileSize = fs->size;
  recieverState.windowCounter = 0;
  recieverState.windowSize = 1;
  recieverState.lastRecievedSeq = -1;
  // Send an ack
  std::unique_ptr<udp_header>k(new udp_header());
  // Any non-0 number indicates ack for fileInfo
  k->ack = 1;
  k->hasFileInfo = true;
  sendHeader(std::move(k),(struct sockaddr *) &recv_addr ,recv_len);

}

void handleSenFStat (udp_header header , char data[PACKET_SIZE] , sockaddr_in recv_addr , socklen_t recv_len) {
  // Thanks for the ack - now senderThread moves on        
  // Acknowledge the ack
  std::unique_ptr<udp_header>k(new udp_header());
  // Any non-0 number indicates ack for fileInfo
  k->ack = -1;
  k->hasFileInfo = true;
  sendHeader(std::move(k),(struct sockaddr *) &recv_addr ,recv_len);
  // TODO need to replace with Seq_max or something
  senderState.expSeqNum = 9999999;
  senderState.windowCounter = 0;
  senderState.lastAckedNum = -1;        
  // Default window Size 
  senderState.windowSize = 1;
  senderState.resume();
}
void handleReciever (udp_header header , char data[PACKET_SIZE] , sockaddr_in recv_addr , socklen_t recv_len) {
  // Anything with ack = 0 mean new data for reciever       
  recieverState.windowSize = header.window_size;
  recieverState.windowCounter--;            
  if (header.seq == recieverState.lastRecievedSeq + 1) {
    // Recieved pakcet properly - lets just increase window size 
    recieverState.lastRecievedSeq = header.seq;
    std::cout << "Sequnce no." << header.seq << std::endl;
    if (data != NULL) {
      // Lets write to a temp file 
      //assembleFile("temp",-1,data,false);
      //std::cout << data.get() << std::endl;
    };
    recieverState.windowCounter++;
    // Send out an Acknowledge when counter > win Size/2
    if (recieverState.windowCounter > recieverState.windowSize / 2) {
      // Send an ack for half the window size
      std::unique_ptr<udp_header>k(new udp_header());
      k->ack = 2;
      k->seq = recieverState.lastRecievedSeq + 1;
      sendHeader(std::move(k),(struct sockaddr *) &recv_addr ,recv_len);
    }
  } else if (header.seq < recieverState.lastRecievedSeq + 1) {
    // Discard the packet but,
    // Send an ack - Maybe the sender didn't recieve it
    // but indicate that this is not a duplicate ack
    std::cout << "Resending ack for seq "<<header.seq << std::endl;
    std::unique_ptr<udp_header>k(new udp_header());
    k->ack = 2;
    k->seq = recieverState.lastRecievedSeq + 1;
    sendHeader(std::move(k),(struct sockaddr *) &recv_addr ,recv_len);
  } else {            
    std::cout << "Duplicate ack " << header.seq<< std::endl;
    // Send a repeat request for the out of order packet
    std::unique_ptr<udp_header>k(new udp_header());
    k->ack = 1;
    k->seq = recieverState.lastRecievedSeq + 1;
    sendHeader(std::move(k),(struct sockaddr *) &recv_addr ,recv_len);
  }
}

void handleSender (udp_header header , char data[PACKET_SIZE] , sockaddr_in recv_addr , socklen_t recv_len) {
  std::cout << "Got ack "<<header.seq << " : "<<senderState.lastAckedNum << std::endl;
  if (header.ack == 1 && senderState.lastAckedNum > header.seq-1) {
    std::cout << "Duplicate Ack" << header.seq << std::endl;
    // Then this is a repeat ack 
    // reset the expected seqnum
    senderState.lastAckedNum = header.seq-1;
    senderState.expSeqNum = header.seq;
    senderState.resume();   
  } else {
    int skip = header.seq - senderState.lastAckedNum;
    senderState.lastAckedNum = header.seq - 1;
    std::cout << "Ack accepted " << header.seq << " Skipping " << skip << std::endl;
    // Either reduce by skip or make it 0 - window can't be negative       
    senderState.windowCounter -= senderState.windowCounter - skip > 0 ? skip : senderState.windowCounter;
    // Got an ack - Lets increase window size - Additive increase
    if (senderState.windowSize < app.max_window_size)
      senderState.windowSize += 1;
    // Then this is an ack , send it to the senderThread if window is full
    //if (senderState.windowCounter > senderState.windowSize) {
    senderState.resume();
  }
}

// A recieved which parses this data 
void* createReciever (void* args) {
  int port = app.recieve_port;
  int s = app.socket;
  // Bind listner socket
  std::cout << "Using port " <<0 << std::endl;
  if (bindUdpSocket(port,s) != -1) {
    printf("OK: bind SUCCESS\n");
  } else {
    printf("Error: bind FAILED\n");
  }
          
  char buffer[PACKET_SIZE];
  std::unique_ptr<char>data(new char[PACKET_SIZE]);
  struct sockaddr_in recv_addr;   udp_header header;
  socklen_t recv_len = (socklen_t)sizeof(recv_addr);
  int count = 0;
  // Reciever vars
  recieverState.windowSize = 1;
  int selectN;
  fd_set readfds;
  struct timeval tv;
  // clear the set ahead of time
  FD_ZERO(&readfds);
  // add our descriptors to the set
  FD_SET(s, &readfds);
  // since we got s2 second, it's the "greater", so we use that for
  // the n param in select()
  selectN = s + 1;
  // wait until either socket has data ready to be recv()d (timeout 10.5 secs)
  tv.tv_sec = 5;
  int rv;
  listen(s, 10);
  long tSize = 0;
  while(1) {
    rv = select(selectN, &readfds, NULL, NULL, &tv);
    tv.tv_sec = 5;
    if (rv == -1) {
      perror("select"); // error occurred in select()
    } else if (rv == 0) {
      // Represents timeout
      if (0 && senderState.isSending) {
        std::cout << "Sender hasn't got an ack" << std::endl;
      }
      if (recieverState.isRecieving){
        std::cout << "Recieve timing out " << std::endl;
        // Send a duplicate ack
        std::unique_ptr<udp_header>k(new udp_header());
        k->ack = 1;
        k->seq = recieverState.lastRecievedSeq + 1;
        sendHeader(std::move(k),(struct sockaddr *) &recieverState.recv_addr ,recieverState.recv_len);
        senderState.setTime();
      }
    } else {
      // one or both of the descriptors have data
      if (FD_ISSET(s, &readfds) && (recvfrom(s,buffer,PACKET_SIZE,0,
                                             (struct sockaddr *) &recv_addr , &recv_len) >= 0)) {
        readPacket(buffer,&header,data.get());
        // Meant for reciever if ack <=0
        if (header.ack <= 0) {
          recieverState.recv_addr = recv_addr;
          recieverState.recv_len = recv_len;
          recieverState.setRTT();
        } else {
          senderState.recv_addr = recv_addr;
          senderState.recv_len = recv_len;
          senderState.setRTT();
        }

        if (header.hasFileInfo) {
          std::cout << "Recieving file info " << std::endl;          
          if (header.ack <= 0) {
            handleRecFStat(header,data.get(),recv_addr , recv_len);
          } else if (header.ack == -1) {
            // Future - Ideally should wait for ack-ack
          } else {     
            handleSenFStat(header,data.get(),recv_addr , recv_len);
          }
        }
        else if (header.ack > 0) {
          handleSender(header,data.get(),recv_addr , recv_len);
        } else if (header.ack <= 0) {
          handleReciever(header,data.get(),recv_addr , recv_len);
        }
        // Blank the buffer
        bzero(buffer, sizeof(buffer));       
      }
    }
  }
  return 0;
}

struct CbArgs {
  sockaddr_in clientaddr;
};

int processBuffer(std::unique_ptr<func_args> args) {
  if (senderState.expSeqNum < senderState.lastAckedNum) {
    // Then goback N
    return senderState.expSeqNum;
  };
  CbArgs *forw = (CbArgs*)args->forw;
  File_stats *fs = (File_stats*) args->func;
  char packet[PACKET_SIZE];
  udp_header k;
  int error;
  long waitTime ;
  if (fs->isFstat) {
    k.hasFileInfo = true;
    std::unique_ptr<fileInfo> info(new fileInfo());
    info->size = fs->totalSize;
    //info->filename = fs->fpath;
    createRequestPacket(packet,&k,info.get());
    do {
      sendPacket((sockaddr*) &forw->clientaddr ,sizeof(sockaddr_in),packet);    
      // Deafult wait is 2 seconds - Use rtt when it arrives
      waitTime = senderState.rtt > 0 ? senderState.rtt : 2000;
      error = senderState.waitTime(waitTime);
    } while (error == ETIMEDOUT); // Retransmit if error 
    // Ok set window size to 1 and lets start keeping some counter 
    senderState.windowSize = 1;
    // Transfer
    senderState.windowCounter = 0;
  } else {
    // No ack present , represented by setting it to 0
    k.hasFileInfo = false;
    k.ack = 0;
    k.seq = fs->seq;
    if(fs->eof){
      waitTime = senderState.rtt > 0 ? senderState.rtt : 2000;
      error = senderState.waitTime(waitTime);
      if (error && senderState.lastAckedNum < k.seq) {
        std::cout << "resending last state"<< senderState.lastAckedNum << "  " << k.seq << std::endl;
        // Resend from the last acked part
        return senderState.lastAckedNum + 1;
      } else {
        std::cout << "FIle Complete " << std::endl;
        return -1;
      }
    }

    writeToBuffer(packet, &k , fs->data);
    std::cout << "Sequence Num " << k.seq << " Wind "<< senderState.windowCounter << std::endl;
    int n = sendto(app.socket, packet , PACKET_SIZE, 0, (const sockaddr*) &forw->clientaddr ,sizeof(sockaddr_in));
    // Wait if 10 packets sent
    if (senderState.windowCounter > senderState.windowSize) {    
      timespec time;
      // TODO --- replace with jacobs rtt
      error = senderState.waitTime(senderState.rtt);
      if (error == ETIMEDOUT) {
        // Resend from the last acked part
        return senderState.lastAckedNum + 1;
      };
      if (senderState.expSeqNum < senderState.lastAckedNum) {
        // Then goback N and reduce window Size /2 
        senderState.windowSize /= 2;
        return senderState.expSeqNum;
      }
    };
    senderState.windowCounter++;
  }  
  return -1;
}

int sendFileInfo(sockaddr_in clientaddr,string name) {
  int sfd = app.socket;  
  socklen_t clientlen = sizeof(clientaddr);
  long size = getFileSize(name);
  char buffer[PACKET_SIZE];
  bzero(buffer,PACKET_SIZE);
  udp_header header;
  header.hasFileInfo = true;
  header.ack = -1;
  fileInfo finfo,*nf;
  nf = new fileInfo();
  finfo.size = getFileSize(name);
  strcpy(finfo.filename ,"Something");
  createRequestPacket(buffer, &header, &finfo);
  
  char nb[PACKET_SIZE];
  memcpy(nb , buffer, PACKET_SIZE);
  memcpy(nf , &nb[sizeof(udp_header)], sizeof(fileInfo));
  std::cout << "*!@!@!" << std::endl;
  std::cout << nf->filename << std::endl;
  // Parse this buffer
  int error;
  long rtt = senderState.rtt;
  if (senderState.rtt == 0)
   rtt = 2000;
  
  do {
    std::cout << "Sending packet" << std::endl;
    sendPacket((sockaddr*) &clientaddr , clientlen , buffer);
    error = senderState.waitTime(rtt);
  } while(error == ETIMEDOUT);  
}

int sendToClient(sockaddr_in clientaddr,const char *respath , long start , long offset ,int (*cb) (std::unique_ptr<func_args>)) {
  int sfd = app.socket;  
  socklen_t clientlen = sizeof(clientaddr);  
  CbArgs *cbArguments = new CbArgs();  
  cbArguments->clientaddr = clientaddr;
  //getPacketFile (string name,char *data, int start, long offset, bool isLast) 
  char buffer[1500];
  int seq = 0;
  long fsize = getFileSize(string(respath));
  while(getPacketFile(string(respath), buffer , -1 , PACKET_SIZE - sizeof(udp_header) , false) != -1) {
    // Do something with the buffer_size
    seq++;
  }
  // int m = getFile(respath , PACKET_SIZE - sizeof(udp_header) ,start, offset , cb , cbArguments);
  // if (m < 0) {
  //   std::cout << "Failed" << std::endl;
  // }
}


// Threaded apps
void* getDataInThread(void* args) {
  thread_args *somargs = (thread_args*)args;
  int port = somargs->port;
  string ip = somargs->ip;
  sockaddr_in cl_addr = getSocketAddr(ip,port);
  //string fname = somargs->filename; 
  string fname = "../www/file";
  sendFileInfo(cl_addr , fname);    
  sendToClient(cl_addr , "../www/file" , 0 , -1 , processBuffer); 
  return NULL;
}
