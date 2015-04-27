#include "common.cpp"
#ifndef __SOCKETHELPER__
#define __SOCKETHELPER__ 1 
#define MAXEVENTS 164
#define BUFFER_SIZE 512
 
// Socket helpers !!!!!!!!!!!!
static int make_socket_non_blocking (int sfd)
{
  int flags, s;
  flags = fcntl (sfd, F_GETFL, 0);
  if (flags == -1)
      return -1;
  flags |= O_NONBLOCK;
  s = fcntl (sfd, F_SETFL, flags);
  if (s == -1)
    return -1;
  return 0;
}
/* reading waiting errors on the socket
 * return 0 if there's no, 1 otherwise
 */
int socket_check(int fd)
{
   int ret;
   int code;
   socklen_t len = sizeof(int);
   ret = getsockopt(fd, SOL_SOCKET, SO_ERROR, &code, &len);
   if ((ret || code)!= 0)
      return -1;
   return 0;
}

int initSocket() {
  // Create socket 
  int s;			   /* s = socket */
  s = socket(AF_INET, SOCK_STREAM, 0);
  //  Enable reuse
  //int true1 = 1;
  // if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *) &true1,
  //                sizeof(true1)) == -1) {
  //   perror("reuseaddr");
  //   return -1;
  // }
  int on = 1;
  if(ioctl(s, FIONBIO, (char *)&on) < 0){
    exit(EXIT_FAILURE);
  }
  //make_socket_non_blocking (s);
  return s;
};

int bindSocket(int &port , int s) {
  sockaddr_in in_addr;	   /* Structure used for bind() */
  in_addr.sin_family = AF_INET;                   /* Protocol domain */
  in_addr.sin_addr.s_addr = 0;                    /* Use wildcard IP address */  
  in_addr.sin_port = htons((short)port);
  port = in_addr.sin_port;
  return bind(s, (struct sockaddr *)&in_addr, sizeof(in_addr)); 
}

int create_and_bind (int port) {
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  int s,sfd;
  memset(&hints,0,sizeof(struct addrinfo));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  char portStr[10];
  sprintf(portStr,"%d",port);
  s = getaddrinfo(NULL,portStr,&hints,&result);
  if (s!=0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    return -1;
  }
    
  struct sockaddr_in in_addr;	   /* Structure used for bind() */
  in_addr.sin_family = AF_INET;                   /* Protocol domain */
  in_addr.sin_addr.s_addr = 0;                    /* Use wildcard IP address */  
  in_addr.sin_port = htons(port);

    
  for (rp=result;rp!=NULL;rp=rp->ai_next) {     
    sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    // sfd =  socket(AF_INET, SOCK_STREAM, 0);
    if (sfd == -1) continue;
    s = bind(sfd, (struct sockaddr *)&in_addr, sizeof(in_addr));
    //s = bind(sfd,rp->ai_addr,rp->ai_addrlen);
    if (s==0) {
      break;
    } else {
      sfd = -1;
      break;
    }
    //close(sfd);
  }
  if (rp==NULL) {
      fprintf(stderr, "Could not bind\n");
      return -1;
  }
  freeaddrinfo(result);
  return sfd;
}
 
int get_and_bind_socket2(bt_args_t *bt_args) {
  // Bind listner socket
  int port = INIT_PORT;
  int s = -1;
  while (port < MAX_PORT) {
    s = create_and_bind(port);
    if (s > 0) {
      std::cout << "Bound to Port " << port << std::endl;
      bt_args->port = port;
      port = MAX_PORT;
      break;
    }
    port++;
  }
  return s;
}
// Get bind and listen
int get_and_bind_socket(bt_args_t *bt_args) {
  // Bind listner socket
  int port = INIT_PORT;
  while (port < MAX_PORT) {
    int s = initSocket();
    if(bindSocket(port,s) <= -1)
      port++;
    else {
      std::cout << "Bound to port " << port << std::endl;
      bt_args->port = port;  
      int k  = listen(s, MAX_CONNECTIONS);
      if (k < 0) {
        std::cout << "Unable to listen " << std::endl;
        abort();
      } 
      return s;
    }
  }
  return -1;
}
void createSockAddr(int port,sockaddr_in *serv_addr) {
  struct hostent *server;
  string ip = "localhost";
  server = gethostbyname("localhost");
  bzero((char *) serv_addr, sizeof(sockaddr_in));  
  serv_addr->sin_family = AF_INET;
  serv_addr->sin_port = htons(port);
  serv_addr->sin_addr.s_addr = htonl(2130706433);
  //bcopy((char *)server->h_addr, (char *)serv_addr->sin_addr.s_addr, server->h_length);
};

char* createHaveMessage(int ind) {
  bt_msg_t msg;  
  msg.length = 0;
  msg.bt_type = BT_HAVE;
  msg.payload.have = ind;
  char *message = new char[sizeof(msg)];
  memcpy(message,(char*)&msg,sizeof(bt_msg_t));
  return message;  
};
void sendHave(bt_args_t *bt_args, int piece ,int not_s) {
  char* msg = createHaveMessage(piece);
  for (auto it = url_to_socket_map.begin(); it != url_to_socket_map.end(); ++it) {
    int s = it->second;
    if (s != not_s) {
      send(s,msg, sizeof(bt_msg_t),0);
    }
  }
}
int efd = epoll_create1(0);
int receiver_efd = efd;
void  __npoll__(int sfd,int (*cb)(bt_args_t*, vector<char>,int),int (*cb2)(bt_args_t*,int) , bt_args_t *bt_args) {
  if (efd == -1) {
    std::cout << "RE: Fatal Epoll create error " << std::endl;
    abort();
  }
  epoll_event event, events[MAXEVENTS];
  event.data.fd = sfd;
  event.events = EPOLLIN | EPOLLET;
  int s = epoll_ctl(efd,EPOLL_CTL_ADD,sfd,&event);
  if (s == -1) {
    std::cout << "RE: Fatal epoll CTL error " << std::endl;
    abort();
  }  
  int n,i;
  char buf[BUFFER_SIZE];
  while(1) {
    n = epoll_wait (efd,events,MAXEVENTS,1000);
    if (n > 0) 
      std::cout << "R: Got "<< n << " events for "<< sfd << std::endl;
     for (i=0;i<n;i++) {
          if ((events[i].events & EPOLLERR) ||
                (events[i].events & EPOLLHUP) ||
              (!(events[i].events & EPOLLIN))) {
            std::cout << events[i].events << std::endl;
            std::cout << "RE : Error receiving " << std::endl;
          } else if (sfd == events[i].data.fd) {
            while (1) {
              struct sockaddr in_addr;
              socklen_t in_len;
              int infd;
              char hbuf[NI_MAXHOST],sbuf[NI_MAXSERV];
              in_len = sizeof in_addr;
              infd = accept(sfd,&in_addr,&in_len);
              if (infd==-1) {
                if((errno==EAGAIN) || (errno == EWOULDBLOCK)) { 
                  break;
                } else {
                  std::cout << "RE: Accept Error " << std::endl;
                  break;
                }
              }
              s = getnameinfo(&in_addr,in_len,hbuf,sizeof hbuf, sbuf,sizeof sbuf, NI_NUMERICHOST | NI_NUMERICSERV);
              url_to_socket_map.insert(std::make_pair("localhost"+ string(sbuf),infd));
              if (s==0) {
                printf("accepted connection on descriptor %d (host=%s, port=%s)\n",infd,hbuf,sbuf);
              }
              s = make_socket_non_blocking(infd);
              if (s==-1) {
                abort();
              }
              event.data.fd = infd;
              event.events = EPOLLIN | EPOLLET;
              s = epoll_ctl(efd,EPOLL_CTL_ADD,infd,&event);
              if (s==-1) {
                perror("epoll_ctl");
                abort();
              }
            }            
          } else {
            vector<char> finalStr;
            finalStr.clear();
            while (1) {
              memset(buf , 0x00 , sizeof(buf));
              ssize_t count; 
              count = recv(events[i].data.fd, buf, BUFFER_SIZE , 0);
              if (count < 0)
                break;
              for (int ii = 0; ii < count; ii++) {
                finalStr.insert(finalStr.end(),buf[ii]);
              }
            }                        
            std::cout << "R: Recieved Data " << finalStr.size();
            // for (auto it = finalStr.begin(); it != finalStr.end(); ++it) {
            //   std::cout<<*it;
            // }
            std::cout<< std::endl;
            cb(bt_args,finalStr,events[i].data.fd);
          }
     }
     // Send out haves for pieces which have just completed 
     int compCount = 0, remCount = 0;
     for (auto it = completed_piece_to_socket_map.begin(); it != completed_piece_to_socket_map.end(); ++it) {
       sendHave(bt_args, it->first,it->second);
       compCount++;
     }
     completed_piece_to_socket_map.clear();
     if (piece_to_socket_map.size() == 0) {
       if (compCount > 0) {
         // I am a seeder now
         closeFile(string(bt_args->save_file));
         std::cout << "Download finished - Will Seed now" << std::endl;
         sleep(4);
       }
     }
     // Send request for pieces which ever are required
     for (auto it = socket_to_piecelist_map.begin(); it != socket_to_piecelist_map.end(); ++it) {
       int s = it->first;
       remCount++;
       cb2(bt_args,s);
     }
     
  }
}

int getSocketPort(int s) {
  sockaddr_in adr;
  socklen_t adr_l;
  int n = getsockname(s,(sockaddr*)&adr , &adr_l);
  if (n == 0) {
    return adr.sin_port;
  } else 
    return -1;
}

/* create a TCP socket with non blocking options and connect it to the target
* if succeed, add the socket in the epoll list and exit with 0
*/
int create_and_connect( sockaddr_in* target , int epfd , int port)
{
   int yes = 1;
   int sock;

   // epoll mask that contain the list of epoll events attached to a network socket
   static struct epoll_event Edgvent;


   if( (sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
   {
      perror("socket");
      exit(1);
   }

   // set socket to non blocking and allow port reuse
   if ( (setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) ||
      fcntl(sock, F_SETFL, O_NONBLOCK)) == -1)
   {
      perror("setsockopt || fcntl");
      exit(1);
   }
   if( connect(sock, (struct sockaddr *)target, sizeof(struct sockaddr)) == -1
      && errno != EINPROGRESS)
   {
      // connect doesn't work, are we running out of available ports ? if yes, destruct the socket
      if (errno == EAGAIN)
      {
         perror("connect is EAGAIN");
         close(sock);
         exit(1);
      }
   }
   else
   {
      Edgvent.events = EPOLLOUT | EPOLLRDHUP | EPOLLET ;
      Edgvent.data.fd = sock;
      // add the socket to the epoll file descriptors
      if(epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &Edgvent) != 0)
      {
         perror("epoll_ctl, adding socket\n");
         exit(1);
      }
   }
   url_to_socket_map.insert(std::make_pair(string("localhost:")+std::to_string(port), sock));
   return 0;
}

#endif
