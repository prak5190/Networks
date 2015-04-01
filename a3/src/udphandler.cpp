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
  //header.isRequest = true;
  createRequestPacket(packet , &header , &info);
  // Send the ack
  //sendto(app.socket, packet , PACKET_SIZE, 0, (const sockaddr*) &forw->clientaddr ,sizeof(sockaddr_in))
  return sendto(app.socket,packet , PACKET_SIZE, 0, addr ,size);
}

void handleRecFStat (udp_header header , char* data , sockaddr_in recv_addr , socklen_t recv_len) {
  fileInfo fs; // = new fileInfo();  
  memcpy(&fs, data , sizeof(fileInfo));   
  recieverState.setRTT();
  recieverState.filename = string(fs.filename);
  recieverState.initialFileSize = recieverState.fileSize = fs.size;
  recieverState.isRecieving = true;
  recieverState.windowCounter = 0;
  recieverState.windowSize = 1;
  recieverState.lastRecievedSeq = -1;
  // Send an ack
  std::unique_ptr<udp_header>k(new udp_header());
  // Any non-0 number indicates ack for fileInfo
  k->ack = AckCodes::ShasFileInfo;
  sendHeader(std::move(k),(struct sockaddr *) &recv_addr ,recv_len);
}

void handleSenFStat (udp_header header , char data[PACKET_SIZE] , sockaddr_in recv_addr , socklen_t recv_len) {
  // Thanks for the ack - now senderThread moves on        
  // Acknowledge the ack
  senderState.setRTT();
  std::unique_ptr<udp_header>k(new udp_header());
  // k->ack = -1;
  // sendHeader(std::move(k),(struct sockaddr *) &recv_addr ,recv_len);
  // TODO need to replace with Seq_max or something
  senderState.expSeqNum = 99999999;
  senderState.windowCounter = 0;
  senderState.lastAckedNum = -1;        
  senderState.isSending = true;
  // Default window Size 
  senderState.windowSize = 1;
  senderState.resume();
}

void handleRecieverSelective (udp_header header , char data[PACKET_SIZE] , sockaddr_in recv_addr , socklen_t recv_len) { 
  
}
void handleReciever (udp_header header , char data[PACKET_SIZE] , sockaddr_in recv_addr , socklen_t recv_len) { 
  if (log(3))
    std::cout << "Header seq " << header.seq << " Next " << recieverState.lastRecievedSeq + 1<<std::endl; 

  if (app.hasDrops) {
    
    srandom(clock());
    // If random num is greater than 100 * prob
    if (random() %100 < (app.drop_probability * 100)) {
      // Drop packet
      if(log(4.5))
        std::cout << "Dropping packet "<< header.seq << std::endl;
      return;
    }
  }
  
  recieverState.windowSize = header.window_size;
  recieverState.windowCounter--;
  if (header.seq == recieverState.lastRecievedSeq + 1) {
    // Set RTT only for each succesful message
    recieverState.setRTT();
    int writeSize = DATA_SIZE ;
    if (recieverState.fileSize < DATA_SIZE)
      writeSize = recieverState.fileSize;
    if (log(3))
      std::cout << "Sequnce1 no." << header.seq  << " : " << recieverState.fileSize << std::endl;
    if (recieverState.fileSize > 0) {
      // Recieved pakcet properly - lets just increase window size 
      recieverState.lastRecievedSeq++;
      if (log(4))
        std::cout << "Sequnce no." << header.seq  << " : " << recieverState.fileSize << std::endl;
      if (data != NULL) {
        // Lets write to a temp file 
        assembleFile("temp",-1,data,writeSize,false);
        recieverState.fileSize -= writeSize;
      };      
      printRecProgress();
      recieverState.windowCounter++;
      // Send out an Acknowledge when counter > win Size/2
      if (recieverState.windowCounter > recieverState.windowSize / 2 || recieverState.fileSize <= 0) {
        // Send an ack for half the window size
        std::unique_ptr<udp_header>k(new udp_header());
        k->ack = AckCodes::SsuccessAck;
        k->seq = recieverState.lastRecievedSeq + 1;
        sendHeader(std::move(k),(struct sockaddr *) &recv_addr ,recv_len);
      }
      if (recieverState.fileSize <= 0) {
        if (!recieverState.fileClosed){
          if(log(6))
            std::cout << "Download finished " << std::endl;
          recieverState.fileClosed = true;          
          closeFile("temp");
        }
      }
    } else {
      std::unique_ptr<udp_header>k(new udp_header());
      k->ack = AckCodes::SsuccessAck;
      k->seq = header.seq + 1;
      sendHeader(std::move(k),(struct sockaddr *) &recv_addr ,recv_len);
      if (!recieverState.fileClosed) {
        if(log(6))
          std::cout << "Download finished " << std::endl;
        closeFile("temp");
        recieverState.fileClosed = true;
      }
    }
  } else if (header.seq < recieverState.lastRecievedSeq + 1) {
    // Discard the packet but,
    // Send an ack - Maybe the sender didn't recieve it
    // but indicate that this is not a duplicate ack
    if(log(4))
      std::cout << "Resending ack for seq "<<header.seq << std::endl;
    std::unique_ptr<udp_header>k(new udp_header());
    k->ack = AckCodes::SrepeatAck;
    k->seq = recieverState.lastRecievedSeq + 1;
    sendHeader(std::move(k),(struct sockaddr *) &recv_addr ,recv_len);
  } else {            
    if(log(4))
      std::cout << "Sending Duplicate ack for " << header.seq<< ": " << recieverState.lastRecievedSeq +1 << std::endl;
    // Send a repeat request for the out of order packet
    std::unique_ptr<udp_header>k(new udp_header());
    k->ack = AckCodes::SduplicateAck;
    k->seq = recieverState.lastRecievedSeq + 1;
    sendHeader(std::move(k),(struct sockaddr *) &recv_addr ,recv_len);
  }
}

void handleSenderSelective (udp_header header , char data[PACKET_SIZE] , sockaddr_in recv_addr , socklen_t recv_len) {
}
void handleSender (udp_header header , char data[PACKET_SIZE] , sockaddr_in recv_addr , socklen_t recv_len) {
  if (log(4))
    std::cout << "Got ack "<<header.seq << " : "<<senderState.lastAckedNum << std::endl;
  if (header.seq >= senderState.lastAckedNum + 1) {
    int skip = header.seq - senderState.lastAckedNum;
    senderState.lastAckedNum = header.seq - 1;
    // Refresh RTT for successful ack
    senderState.setRTT();
    if(log(4))
      std::cout << "Ack accepted " << header.seq << " Skipping " << skip << std::endl;
    // Either reduce by skip or make it 0 - window can't be negative       
    senderState.windowCounter -= senderState.windowCounter - skip > 0 ? skip : senderState.windowCounter;
    // Got an ack - Lets increase window size - Additive increase
    if (senderState.windowSize < app.max_window_size) {
      if (senderState.isAimdorSlow)
        senderState.windowSize += 1;
      else // Time for some exponential increase -> slow start
        senderState.windowSize += senderState.windowSize;
      if (senderState.windowSize > app.max_window_size) 
        senderState.windowSize = app.max_window_size;
    }
    printSenderStatistics();
    // Then this is an ack , send it to the senderThread if window is full
    //if (senderState.windowCounter > senderState.windowSize) {
    senderState.resume();
  } else if (header.ack == AckCodes::SduplicateAck && senderState.lastAckedNum > header.seq-1) {
    if(log(4.5))
      std::cout << "Duplicate Ack" << header.seq << std::endl;
    // Then this is a repeat ack 
    // reset the expected seqnum
    senderState.lastAckedNum = header.seq-1;
    senderState.expSeqNum = header.seq;
    if (log(3.4))
      std::cout << "Doing multiplicative decrease " << std::endl;
    senderState.handleDrop();
  } else {
    // Already ack recieved for this -- Just a repeat ack - ignore
  }
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
  std::unique_ptr<char>data(new char[PACKET_SIZE]);
  struct sockaddr_in recv_addr;   
  udp_header header;
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
    
    // Give random delays using sleep -- Happens on both server and client side
    if (app.hasLatency) {
      if(log(4.5))
        std::cout << "Creating latency " << std::endl;
      srandom(clock());
      usleep(1000* (random() % 10) * (app.latencyTime / 10));
    };

    if (rv == -1) {
      perror("select"); // error occurred in select()
    } else if (rv == 0) {
      // Represents timeout
      if (senderState.isSending) {
        if (log(5))
          std::cout << "Sender timing out" << std::endl;
        senderState.handleDrop();
      }
      if (recieverState.isRecieving){
        if(log(5))
          std::cout << "Recieve timing out " << std::endl;
        // Send a duplicate ack
        std::unique_ptr<udp_header>k(new udp_header());
        k->ack = AckCodes::SduplicateAck;
        k->seq = recieverState.lastRecievedSeq + 1;
        sendHeader(std::move(k),(struct sockaddr *) &recieverState.recv_addr ,recieverState.recv_len);
        recieverState.setTime();
      }
    } else {
      // one or both of the descriptors have data
      if (FD_ISSET(s, &readfds) && (recvfrom(s,buffer,PACKET_SIZE,0,
                                             (struct sockaddr *) &recv_addr , &recv_len) >= 0)) {
        readPacket(buffer,&header,data.get());        
        // Meant for reciever if ack <=0
        if (header.ack == AckCodes::RData || header.ack == AckCodes::RhasFileInfo) {
          recieverState.recv_addr = recv_addr;
          recieverState.recv_len = recv_len;
        } else {
          senderState.recv_addr = recv_addr;
          senderState.recv_len = recv_len;
        }
        switch (header.ack) {
        case AckCodes::RhasFileInfo : handleRecFStat(header,data.get(),recv_addr , recv_len);
          break;
        case AckCodes::ShasFileInfo : handleSenFStat(header,data.get(),recv_addr , recv_len);
          break;
        case AckCodes::RData : handleReciever(header,data.get(),recv_addr , recv_len);
          break;
        case AckCodes::SrepeatAck: 
        case AckCodes::SsuccessAck:
        case AckCodes::SwrapSequence:
        case AckCodes::SduplicateAck :          
          handleSender(header,data.get(),recv_addr , recv_len);
          break;
        };
        // Blank the buffer
        bzero(buffer, sizeof(buffer));       
      }
    }
  }
  return 0;
}

int sendBuffer(sockaddr_in clientaddr,char* data , int seq) {
  if (senderState.expSeqNum < senderState.lastAckedNum) {
    // Then goback N
    return senderState.expSeqNum;
  } else if (senderState.expSeqNum == senderState.lastAckedNum) {
    senderState.expSeqNum = senderState.lastAckedNum + 1;    
  }
  if (seq < senderState.lastAckedNum + 1) {
    // Don't resend old - send new 
    return senderState.lastAckedNum + 1;
  };  
  char packet[PACKET_SIZE];
  udp_header k;
  long waitTime ;
  // No ack present , represented by setting it to 0
  //k.hasFileInfo = false;
  k.ack = AckCodes::RData;
  k.seq = seq;  
  writeToBuffer(packet, &k , data);
  if(log(4))
    std::cout << "Sequence Num " << k.seq << " Wind "<< senderState.windowCounter << std::endl;
  int n = sendto(app.socket, packet , PACKET_SIZE, 0, (const sockaddr*) &clientaddr ,sizeof(sockaddr_in));
  senderState.windowCounter++;   
  return -1;
}

int sendFileInfo(sockaddr_in clientaddr,string name) {
  int sfd = app.socket;  
  socklen_t clientlen = sizeof(clientaddr);
  long size = getFileSize(name);
  if (size == -1) {
    return -1;
  }
  char buffer[PACKET_SIZE];
  bzero(buffer,PACKET_SIZE);
  udp_header header;
  header.ack = AckCodes::RhasFileInfo;
  fileInfo finfo,*nf;
  nf = new fileInfo();
  finfo.size = size;
  strcpy(finfo.filename ,"Something");
  createRequestPacket(buffer, &header, &finfo);
  
  char nb[PACKET_SIZE];
  memcpy(nb , buffer, PACKET_SIZE);
  memcpy(nf , &nb[sizeof(udp_header)], sizeof(fileInfo));
  // Parse this buffer
  int error;
  long rtt = senderState.rtt;
  if (senderState.rtt == 0)
   rtt = 2000;  
  do {
    sendPacket((sockaddr*) &clientaddr , clientlen , buffer);
    error = senderState.waitTime(rtt);
  } while(error == ETIMEDOUT);  
  if(log(4))
    std::cout << "Got ack from reciever " << std::endl;
  // Ok set window size to 1 and lets start keeping some counter 
  senderState.windowSize = 1;
  // Transfer
  senderState.windowCounter = 0;
  return 0;
}

int sendToClient(sockaddr_in clientaddr,const char *respath , long start , long offset) {
  if(log(6))
    std::cout << "Sending data " << std::endl;
  int sfd = app.socket;  
  socklen_t clientlen = sizeof(clientaddr);  
  //getPacketFile (string name,char *data, int start, long offset, bool isLast) 
  char buffer[DATA_SIZE];
  int seq = 0;
  //long fsize = getFileSize(string(respath));  
  int expSeq;
  int maxSeq = -1;
  int error;
  while(1) {
    if (getPacketFile(string(respath), buffer , seq * DATA_SIZE , DATA_SIZE , false) == -1) {
      maxSeq = seq;
      if(log(3))
        std::cout << "Max sequence is " <<seq << std::endl;
    }
    // Do something with the buffer_size
    expSeq = sendBuffer(clientaddr,buffer, seq);
    if (senderState.windowCounter > senderState.windowSize) {    
      timespec time;  
      if(log(4.5))
        std::cout << "Sender RTT " << senderState.rtt << " : WIndow Size " << senderState.windowSize <<" : Win Counter " << senderState.windowCounter << std::endl;
      error = senderState.waitTime(senderState.rtt);
      if (error == ETIMEDOUT) {
        senderState.handleDrop();
        // Resend from the last acked part
        expSeq = senderState.lastAckedNum + 1;
      };
      if (senderState.expSeqNum < senderState.lastAckedNum) {
        expSeq =  senderState.expSeqNum;
      }
      continue;
    };

    if(log(3))
    if (seq == maxSeq) {
      std::cout << "LAst seq " << std::endl;
    }
    if (expSeq == -1 && (seq < maxSeq || maxSeq == -1 )) 
      seq++;
    else {
      seq = expSeq;
    }
    if(log(4))
      std::cout << "Last acked Num " << senderState.lastAckedNum << std::endl;
    if (senderState.lastAckedNum >= maxSeq && maxSeq != -1) 
      break;
  }
  if(log(6))
    std::cout << "Finished " << std::endl;
  exit(0);
}

// Threaded apps
void* getDataInThread(void* args) {
  thread_args *somargs = (thread_args*)args;
  int port = somargs->port;
  string ip = somargs->ip;
  sockaddr_in cl_addr = getSocketAddr(ip,port);
  //string fname = "../www/a.mp3";
  string fname = "./www/";
  fname.append(somargs->filename); 
  if(log(6))
    std::cout << "Sending File : "<< fname << std::endl;
  if(sendFileInfo(cl_addr , fname) == -1) {
    if(log(7))
      std::cout << "File not found " << std::endl;    
  } else {
    sendToClient(cl_addr , fname.c_str() , 0 , -1); 
  }   
  return NULL;
}
