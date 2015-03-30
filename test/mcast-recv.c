/*
   mcast-r.c: multicast receiver
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MAXSIZE 100

int main(int argc, char *argv[])
{
   int s;			   /* s = socket */
   struct sockaddr_in in_addr;	   /* Structure used for bind() */
   struct sockaddr_in sock_addr;   /* Output structure from getsockname */
   struct sockaddr_in src_addr;    /* Used to receive (addr,port) of sender */
   int src_addr_len;		   /* Length of src_addr */
   int len;			   /* Length of result from getsockname */
   int mc_addr, port;
   struct ip_mreq mreq;
   struct hostent *host_entry_ptr;
   char line[MAXSIZE];


   if (argc < 3)
    { printf("Usage: %s mc_addr port\n", argv[0]);
      printf("Note: use udp4-s2 to send on multicast address\n");
      exit(1);
    }

   mc_addr = inet_addr(argv[1]);
   port = atoi(argv[2]);

   /* ---------------------------
      Create a socket
      --------------------------- */
   s = socket(AF_INET, SOCK_DGRAM, 0);

  /* -------------------------
     Enable reuse
     ------------------------- */
{  int true = 1;

   if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *) &true,
                  sizeof(true)) == -1)
    { perror("reuseaddr");
      return(-1);
    }
}

   /* ------------------------------------------
      Set up socket end-point info for binding
      ------------------------------------------ */
   in_addr.sin_family = AF_INET;                   /* Protocol domain */
   in_addr.sin_addr.s_addr = 0;                    /* Use wildcard IP address */
   in_addr.sin_port = htons(port);	       	   /* Use this UDP port */

   /* ---
      Here goes the binding...
      --- */
   if (bind(s, (struct sockaddr *)&in_addr, sizeof(in_addr)) == -1)
    { printf("Error: bind FAILED\n");
      exit(1);
    }
   else
    { printf("OK: bind SUCCESS\n");
    }

  /* =======================================================================
     This is needed to change the UDP IP-address to an IP-multicast address
     ======================================================================= */
  /* ----
     Setup multicast enroll information in `mreq'
     ---- */
  mreq.imr_multiaddr.s_addr = mc_addr;
  mreq.imr_interface.s_addr = htonl(INADDR_ANY);

  /* ----------------------------------------------------------
     Add multicast membership...
     ---------------------------------------------------------- */
  if (setsockopt(s, IPPROTO_IP, IP_ADD_MEMBERSHIP,(char *) &mreq, sizeof(mreq))
		== -1)
   { printf("Error membership...\n");
     exit(1);
   }

   /* ===========================================
      We are now ready for multicast packets
      =========================================== */

   while(1)
    { src_addr_len = sizeof(src_addr);
      len = recvfrom(s, line, MAXSIZE, 0 /* flags */,
                    (struct sockaddr *) &src_addr, &src_addr_len);

      /* ==================================================== Changed ... */
      host_entry_ptr = gethostbyaddr((char *) &(src_addr.sin_addr.s_addr), 
				     sizeof(src_addr.sin_addr.s_addr), AF_INET);

      printf("Msg from (%s,%u): `%s' (%u bytes)\n", 
		host_entry_ptr->h_name, src_addr.sin_port, line, len);
    }
}
