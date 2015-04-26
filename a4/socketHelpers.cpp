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
        sleep(10);
        abort();
      } 
      return s;
    }
  }
  return -1;
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
     // Send request for pieces which ever are required
     for (auto it = socket_to_piecelist_map.begin(); it != socket_to_piecelist_map.end(); ++it) {
       int s = it->first;
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

/* create a TCP socket with non blocking options and connect it to the target
* if succeed, add the socket in the epoll list and exit with 0
*/
int connect_to_addr(sockaddr_in *target , int epfd)
{
   int yes = 1;
   int sock;

   // epoll mask that contain the list of epoll events attached to a network socket
   static struct epoll_event Edgvent; 
   if(connect(sock,(struct sockaddr *)target, sizeof(struct sockaddr)) == -1 && errno != EINPROGRESS)
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
     Edgvent.events =  EPOLLOUT | EPOLLRDHUP | EPOLLERR | EPOLLET ;     
     Edgvent.data.fd = sock;     
     // add the socket to the epoll file descriptors
     if(epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &Edgvent) != 0)
       {
         perror("epoll_ctl, adding socket\n");
         exit(1);
       }
   }   
   return 0;
}

// int setReadPoll(int efd,int fd) {
//   epoll_event event;
//   event.data.fd = fd;
//   event.events = EPOLLIN;
//   return epoll_ctl (efd, EPOLL_CTL_MOD, fd, &event);  
// }
// int setWritePoll(int efd,int fd) {
//   epoll_event event;
//   event.data.fd = fd;
//   event.events = EPOLLOUT;
//   return epoll_ctl (efd, EPOLL_CTL_MOD, fd, &event);
// }

// int sender_efd = epoll_create1 (0);
// void __poll__(int sfd,int (*cb)(bt_args_t*,const char*,int,int) , bt_args_t *bt_args) {
//   int s = listen (sfd, SOMAXCONN); 
//   int efd = epoll_create1 (0);
//   if (efd == -1) {    
//     std::cout << "RE: Some issue in polling " << strerror(errno) << std::endl;
//     // Error out 
//     return;
//   }
//   if (s == -1) {
//     std::cout << "RE: Socket issue" << strerror(errno) << std::endl;
//     return;
//   }
//   struct epoll_event event;
//   event.data.fd = sfd;
//   event.events = EPOLLIN | EPOLLET;
//   s = epoll_ctl (efd, EPOLL_CTL_ADD, sfd, &event);  
//   struct epoll_event *events;
//   events = new epoll_event[MAXEVENTS];  
//   char *buf = new char[BUFFER_SIZE];
//   /* The event loop */
//   while (1) {
//     int n, i;        
//     n = epoll_wait (efd, events, MAXEVENTS, 1000);    
//     if (n>0)
//       std::cout << "R: Got Events on " << n <<" items where sfd is "<< sfd << std::endl; 
//     for (i = 0; i < n; i++) {      
//       if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) || (events[i].events & EPOLLRDHUP)) {
//         // Error occurred 
//         std::cout << "R: Error in EPOLL " << std::endl;
//       } else if (events[i].events & EPOLLOUT) {
//         int s = events[i].data.fd;
//         std::cout << "R: Sending some data " << s << std::endl;
//         int a1 = send(s,"Fuckkkkkkkkkk111111111111111111111111111111111111U",200,MSG_DONTWAIT);
//         if (a1 > 0) {   
//           std::cout << a1 << "Succcesssssssssssssssssssssssssssssssssssssssss  " << s << std::endl;    
//         } else {
//           std::cout << "Failureeeeeeeeeeeeeee" << strerror(errno) << std::endl;
//         }
//         setReadPoll(efd, s);          
//         event.data.fd = s;
//         event.events = EPOLLIN;        
//         int m  = epoll_ctl (efd, EPOLL_CTL_MOD, s, &event);   
//         if (m == 0) 
//           std::cout << "********* !!!!!!!!!!!!!!! " << std::endl;
//         else
//           std::cout << "***** ! " << strerror(errno) << std::endl;
//         std::cout << "EVents on "<<events[i].data.fd << "  " << (events[i].events & EPOLLIN) << std::endl;
//       } else if (sfd == events[i].data.fd) {
//         std::cout << "R: Binding clients " << std::endl; 
//         while (1) {
//           struct sockaddr in_addr;
//           socklen_t in_len;
//           int infd;
//           char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
          
//           in_len = sizeof in_addr;
//           //infd = accept (sfd, &in_addr, &in_len);
//           infd = accept4(sfd, &in_addr, &in_len, SOCK_NONBLOCK);
//           if (infd == -1) {
//             if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
//               /* We have processed all incoming connections. */
//               std::cout << "R: Processed everything" << std::endl;
//               break;
//             } else {              
//               break;
//             }
//           }          
//           s = getnameinfo (&in_addr, in_len, hbuf, sizeof hbuf,sbuf, sizeof sbuf, NI_NUMERICHOST | NI_NUMERICSERV);
//           if (s == 0) 
//             printf("Accepted connection on descriptor %d (host=%s, port=%s)\n", infd, hbuf, sbuf);                  
          
//           /* Make the incoming socket non-blocking and add it to the list of fds to monitor. */          
//           s = make_socket_non_blocking (infd);
//           if (s == -1)
//             abort ();     
//           epoll_event *even = new epoll_event();     
//           even->data.fd = infd;
//           even->events = EPOLLIN;
//           s = epoll_ctl (efd, EPOLL_CTL_ADD, infd, even);                    

//           epoll_event even2;
//           even2.data.fd = infd;
//           even2.events = EPOLLOUT;
//           s = epoll_ctl (sender_efd, EPOLL_CTL_ADD, infd, &even2);                     
//           std::cout << "********** "<<infd << " added to sender_efd" << std::endl;
//           if (s == -1) {
//             perror ("epoll_ctl");
//             abort ();
//           }
//         }
//       } else {
         
//         // Get data - Lets collect it , set EPOLLOUT and call cb when EPOLLOUT comes
//         int done = 0;
//         int type = -1;
//         int count;
//         string finalStr = "";
//         while (1) {
//           memset(buf , 0x00 , sizeof(buf));
//           ssize_t count;
//           count = recv(events[i].data.fd, buf, BUFFER_SIZE , 0);
//           finalStr += buf;
//           if (count < 0)
//             break;                    
//         }
//         std::cout << "R: Recieved Data "<<finalStr << std::endl;
//         int s = events[i].data.fd;
//         //setWritePoll(efd,events[i].data.fd);
//       }
        
// //           ssize_t count;
// //           count = read(events[i].data.fd, buf, BUFFER_SIZE);
// //           if (count == -1) {
// //             /* If errno == EAGAIN, that means we have read all
// //                data. So go back to the main loop. */
// //             if (errno != EAGAIN) {
// //               perror ("read");
// //               done = 1;
// //             }
// //             break;
// //           } else if (count == 0) {
// //             /* End of file. The remote has closed the
// //                connection. */
// //             done = 1;
// //             break;
// //           }
// //           /* Write the buffer to standard output */
// //           write (1, buf, count);
// //           std::cout << "********************************************************" << std::endl; 
// //           std::cout << "R: Got Buffer " << buf << std::endl;
// //           if (buf[0] == 19) {
// //             type = 0;
// //           } else {             
// //             type = 1;     
// //           }
// //           finalStr = finalStr + string(buf,BUFFER_SIZE);
// //           // char *dup = new char[BUFFER_SIZE];
// //           // memcpy(dup , buf , BUFFER_SIZE);
// //           // cb(dup,type,events[i].data.fd);
// //           if (s == -1) {
// //             perror ("write");
// //             abort ();
// //           }
// //         }          

//     }
//   }
// }
// //int (*cb)(bt_args_t*,const char*,int,int) ,

// void __spoll__(int sfd, bt_args_t *bt_args) {
//   int efd = sender_efd; 
//   int s = listen (sfd, SOMAXCONN); 
//   if (efd == -1) {    
//     std::cout << "RE: Some issue in polling " << strerror(errno) << std::endl;
//     // Error out 
//     return;
//   }
//   if (s == -1) {
//     std::cout << "RE: Socket issue" << strerror(errno) << std::endl;
//     return;
//   }
//   struct epoll_event event;
//   event.data.fd = sfd;
//   event.events = EPOLLOUT;
//   s = epoll_ctl (efd, EPOLL_CTL_ADD, sfd, &event);  
//   struct epoll_event *events;
//   events = new epoll_event[MAXEVENTS];  
//   char *buf = new char[BUFFER_SIZE];
//   /* The event loop */
//   while (1) {
//     int n, i;        
//     n = epoll_wait (efd, events, MAXEVENTS, 1000);    
//     if (n > 0)
//       std::cout << "H: Got Events on " << n <<" items" << std::endl; 
//     for (i = 0; i < n; i++) {      
//       if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) || (events[i].events & EPOLLRDHUP)) {
//         // Error occurred 
//         std::cout << "H: Error in EPOLL " << std::endl;
//       } else if (events[i].events & EPOLLOUT) {
//         int s = events[i].data.fd;
//         std::cout << "H: Can Write on" << s << std::endl;
//         int a1 = send(s,"Fuckkkk1kkkkkk111111111111111111111111111111111111U",40,MSG_DONTWAIT);
//         if (a1 > 0) { 
//           std::cout << a1 << "H: Succces  " << s << std::endl;    
//         } else {
//           std::cout << "H: Failureeeeeeeeeeeeeee" << strerror(errno) << std::endl;
//         }       

//       }       
//     }
//   }
// }

// void __poll__(int sfd,int (*cb)(bt_args_t*,const char*,int,int) , bt_args_t *bt_args) {
//   int s = listen (sfd, SOMAXCONN); 
//   int efd = epoll_create1 (0);
//   if (efd == -1) {    
//     std::cout << "RE: Some issue in polling " << strerror(errno) << std::endl;
//     // Error out 
//     return;
//   }
//   if (s == -1) {
//     std::cout << "RE: Socket issue" << strerror(errno) << std::endl;
//     return;
  
//   }
//   struct epoll_event event;
//   event.data.fd = sfd;
//   event.events = EPOLLET | EPOLLIN | EPOLLOUT;
//   s = epoll_ctl (efd, EPOLL_CTL_ADD, sfd, &event);  
//   struct epoll_event *events;
//   events = new epoll_event[MAXEVENTS];  
//   char *buf = new char[BUFFER_SIZE];
//   /* The event loop */
//   while (1) {
//     int n, i;        
//     n = epoll_wait (efd, events, MAXEVENTS, 1000);    
//     std::cout << "R: Got Events on " << n <<" items"  << std::endl;
//     int s = events[0].data.fd;
//     int a1 = send(s,"Fuckkkkkkkkkk111111111111111111111111111111111111U",200,0);
//     if (a1 > 0) { 
//       std::cout << a1 << "Succcesssssssssssssssssssssssssssssssssssssssss  " << s << std::endl;    
//     } else {
//       std::cout << "Failureeeeeeeeeeeeeee" << strerror(errno) << std::endl;
//     }       
//     for (i = 0; i < n; i++) {
//       //(!(events[i].events & EPOLLIN))
//       if((events[i].events & EPOLLOUT)) {
//         // Lets sends some data        
//         std::cout << "*********** Sending data "  << std::endl;
//         //write (1, buf, count);
//         int s = events[i].data.fd;
//         int a1 = send(s,"Fuckkkkkkkkkk111111111111111111111111111111111111U",200,0);
//         if (a1 > 0) { 
//           std::cout << a1 << "Succcesssssssssssssssssssssssssssssssssssssssss  " << s << std::endl;    
//         } else {
//         }       
//           std::cout << "Failureeeeeeeeeeeeeee" << strerror(errno) << std::endl;
//       } 
//       if (events[i].events & EPOLLIN) {
//         std::cout << "Someone sent somethign " << std::endl;
//       }
//       // Use this to detect client timeouts
//       if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) || (events[i].events & EPOLLRDHUP)) {
//         /* An error has occured on this fd, or the socket is not
//            ready for reading (why were we notified then?) */
//         std::cout << "RE: Epoll Error " << std::endl;
//         //close (events[i].data.fd);
//         continue;
//       } else if (sfd == events[i].data.fd) {
//         std::cout << "Came hereeeeeeeeeeeeeeeeeeeeeeeeeeee " << std::endl;
//         /* We have a notification on the listening socket, which
//            means one or more incoming connections. */
//         while (1) {
//           struct sockaddr in_addr;
//           socklen_t in_len;
//           int infd;
//           char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
            
//           in_len = sizeof in_addr;
//           infd = accept4(sfd, NULL, 0, SOCK_NONBLOCK);
//           //infd = accept (sfd, &in_addr, &in_len);
//           if (infd == -1) {
//             if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
//               /* We have processed all incoming
//                  connections. */
//               std::cout << "R: Processed everything" << std::endl;
//               break;
//             }
//             else {
//               perror ("accept");
//               break;
//             }
//           }
            
//           s = getnameinfo (&in_addr, in_len, hbuf, sizeof hbuf,
//                            sbuf, sizeof sbuf, NI_NUMERICHOST | NI_NUMERICSERV);
//           if (s == 0) {
//             printf("Accepted connection on descriptor %d "
//                    "(host=%s, port=%s)\n", infd, hbuf, sbuf);
//           }
            
//           /* Make the incoming socket non-blocking and add it to the
//              list of fds to monitor. */          
//           //s = make_socket_non_blocking (infd);
//           if (s == -1)
//             abort ();
            
//           event.data.fd = infd;
//           event.events = EPOLLIN | EPOLLOUT | EPOLLET;
//           s = epoll_ctl (efd, EPOLL_CTL_ADD, infd, &event);          

//           if (s == -1) {
//             perror ("epoll_ctl");
//             abort ();
//           }
//         }
//         continue;
//       } else {
//         /* We have data on the fd waiting to be read. Read and
//            display it. We must read whatever data is available
//            completely, as we are running in edge-triggered mode
//            and won't get a notification again for the same
//            data. */
//         int done = 0;
//         int type = -1;
//         string finalStr = "";
//         while (1) {
//           memset(buf , 0x00 , sizeof(buf));
          
//           ssize_t count;
//           count = read(events[i].data.fd, buf, BUFFER_SIZE);
//           if (count == -1) {
//             /* If errno == EAGAIN, that means we have read all
//                data. So go back to the main loop. */
//             if (errno != EAGAIN) {
//               perror ("read");
//               done = 1;
//             }
//             break;
//           } else if (count == 0) {
//             /* End of file. The remote has closed the
//                connection. */
//             done = 1;
//             break;
//           }
//           /* Write the buffer to standard output */
//           write (1, buf, count);
//           std::cout << "********************************************************" << std::endl; 
//           std::cout << "R: Got Buffer " << buf << std::endl;
//           if (buf[0] == 19) {
//             type = 0;
//           } else {             
//             type = 1;     
//           }
//           finalStr = finalStr + string(buf,BUFFER_SIZE);
//           // char *dup = new char[BUFFER_SIZE];
//           // memcpy(dup , buf , BUFFER_SIZE);
//           // cb(dup,type,events[i].data.fd);
//           if (s == -1) {
//             perror ("write");
//             abort ();
//           }
//         }          
//         //cb(bt_args,finalStr.c_str(),type,events[i].data.fd);        
//         if (done) {
//           printf ("R: Closed connection on descriptor %d\n", events[i].data.fd);            
//           /* Closing the descriptor will make epoll remove it
//              from the set of descriptors which are monitored. */
//           //close (events[i].data.fd);
//         }
//       }
//     }             
//   }
// }


// // Reads data into udp_header 
// void readPacket(char* buffer , udp_header *header , char* data) {
//   if (buffer == NULL)
//     return;
//   size_t sz = sizeof(udp_header);
//   memcpy(header , buffer , sz);
//   memcpy(data , &buffer[sz]  , PACKET_SIZE - sz);
//   if (log(3.9))
//     std::cout << "Got a header " << header->seq << ": " << (int) header->ack << std::endl;
// };
// void readPacket(char* buffer , udp_header *header , fileInfo *data) {
//   if (buffer == NULL)
//     return;
//   size_t sz = sizeof(udp_header);
//   memcpy(header , buffer , sz);
//   memcpy(data , &buffer[sz]  , sizeof(fileInfo));
// };


// void getFileInfoFromData(char* data, fileInfo *f) {
//   memcpy(f, data , sizeof(fileInfo));  
// }
// void createRequestPacket(char* buffer, udp_header *header, fileInfo *finfo) {
//   size_t sz = sizeof(udp_header);
//   memcpy(buffer, header, sz);
//   memcpy(&buffer[sz] , finfo , sizeof(fileInfo));
// };

// void writeToBuffer(char* buffer, udp_header *header, const char* data) {
//   size_t sz = sizeof(udp_header);
//   memcpy(buffer, header, sz);
//   memcpy(&buffer[sz] , data , PACKET_SIZE - sz);
// };
#endif
