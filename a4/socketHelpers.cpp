#include "common.cpp"
#ifndef __SOCKETHELPER__
#define __SOCKETHELPER__ 1 
#define MAXEVENTS 64
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
  make_socket_non_blocking (s);
  return s;
};

int bindSocket(int port , int s) {
  sockaddr_in in_addr;	   /* Structure used for bind() */
  in_addr.sin_family = AF_INET;                   /* Protocol domain */
  in_addr.sin_addr.s_addr = 0;                    /* Use wildcard IP address */  
  in_addr.sin_port = port;
  return bind(s, (struct sockaddr *)&in_addr, sizeof(in_addr));
}

sockaddr_in getSocketAddr(string ip , int port) {
  struct hostent *server;
  server = gethostbyname(ip.c_str());
  sockaddr_in serv_addr;
  bzero((char *) &serv_addr, sizeof(sockaddr_in));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = port;
  return serv_addr;
}

int get_and_bind_socket(bt_args_t *bt_args) {
  int s = initSocket();
  // Bind listner socket
  int port = INIT_PORT;
  while (bindSocket(port,s) == -1) {
    port++;
  }
  bt_args->port = port;
  std::cout << "Bound to port "<< port << std::endl;
  return s;
}

void __poll__(int sfd,int (*cb)(char*,int,int)) {
  int s = listen (sfd, SOMAXCONN); 
  int efd = epoll_create1 (0);
  if (efd == -1) {    
    std::cout << "Some issue in polling " << strerror(errno) << std::endl;
    // Error out 
    return;
  }
  if (s == -1) {
    std::cout << "Socket issue" << strerror(errno) << std::endl;
    return;
  }
  
  struct epoll_event event;
  event.data.fd = sfd;
  event.events = EPOLLIN | EPOLLET;
  s = epoll_ctl (efd, EPOLL_CTL_ADD, sfd, &event);
  
  struct epoll_event *events;
  /* Buffer where events are returned */
  events = (epoll_event*)calloc (MAXEVENTS, sizeof(epoll_event));
  /* The event loop */
  while (1) {
    int n, i;        
    n = epoll_wait (efd, events, MAXEVENTS, -1);
    for (i = 0; i < n; i++) {
      if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) || (!(events[i].events & EPOLLIN))) {
        /* An error has occured on this fd, or the socket is not
           ready for reading (why were we notified then?) */
        fprintf (stderr, "epoll error\n");  
        close (events[i].data.fd);
        continue;
      }  else if (sfd == events[i].data.fd) {
        /* We have a notification on the listening socket, which
           means one or more incoming connections. */
        while (1) {
          struct sockaddr in_addr;
          socklen_t in_len;
          int infd;
          char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
            
          in_len = sizeof in_addr;
          infd = accept (sfd, &in_addr, &in_len);
          if (infd == -1) {
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
              /* We have processed all incoming
                 connections. */
              std::cout << "Processed everything" << std::endl;
              break;
            }
            else {
              perror ("accept");
              break;
            }
          }
            
          s = getnameinfo (&in_addr, in_len, hbuf, sizeof hbuf,
                           sbuf, sizeof sbuf, NI_NUMERICHOST | NI_NUMERICSERV);
          if (s == 0) {
            printf("Accepted connection on descriptor %d "
                   "(host=%s, port=%s)\n", infd, hbuf, sbuf);
          }
            
          /* Make the incoming socket non-blocking and add it to the
             list of fds to monitor. */
          s = make_socket_non_blocking (infd);
          if (s == -1)
            abort ();
            
          event.data.fd = infd;
          event.events = EPOLLIN | EPOLLET;
          s = epoll_ctl (efd, EPOLL_CTL_ADD, infd, &event);
          if (s == -1) {
            perror ("epoll_ctl");
            abort ();
          }
        }
        continue;
      } else {
        /* We have data on the fd waiting to be read. Read and
           display it. We must read whatever data is available
           completely, as we are running in edge-triggered mode
           and won't get a notification again for the same
           data. */
        int done = 0;          
        bool isData = false;
        while (1) {
          ssize_t count;
          char buf[512];
            
          count = read (events[i].data.fd, buf, sizeof buf);
          if (count == -1) {
            /* If errno == EAGAIN, that means we have read all
               data. So go back to the main loop. */
            if (errno != EAGAIN) {
              perror ("read");
              done = 1;
            }
            break;
          } else if (count == 0) {
            /* End of file. The remote has closed the
               connection. */
            done = 1;
            break;
          }
          /* Write the buffer to standard output */
          //s = write (1, buf, count);
          int type = -1;
          if (buf[0] == 19) {
            type = 0;
          } else {
            if (isData) {
              type = 2;
            } else {
              type = 1;
              isData = true;
            }            
          }
          cb(buf,type,events[i].data.fd);
          if (s == -1) {
            perror ("write");
            abort ();
          }
        }          
        if (done) {
          printf ("Closed connection on descriptor %d\n", events[i].data.fd);            
          /* Closing the descriptor will make epoll remove it
             from the set of descriptors which are monitored. */
          close (events[i].data.fd);
        }
      }
    }             
  }
}


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
