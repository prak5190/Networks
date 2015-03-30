/*
   mcast-send.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

int main(int argc, char *argv[])
{
   int s;				/* s = socket */
   struct sockaddr_in in_addr;		/* Structure used for bind() */
   struct sockaddr_in sock_addr;	/* Output structure from getsockname */
   struct sockaddr_in dest_addr;	/* Destination socket */
   char line[100];
   int len;

   if (argc < 3)
    { printf("Usage: %s <dotted-dec-dest-addr> <dest-port>\n", argv[0]);
      printf(" Program sends messages to <dotted-dec-dest-addr> <dest-port>\n");
      printf(" Use `host' to find dotted dec. address\n");
      exit(1);
    }

   /* -------
      Fill in destination socket - this will identify IP-host + (UDP) port
      ------- */
   dest_addr.sin_family = AF_INET;		 /* Internet domain */

   /**********************************************************************/
   /* Note: inet_addr() returns Internet address in Network byte order   */
   /**********************************************************************/
   if ( (dest_addr.sin_addr.s_addr = inet_addr(argv[1])) == -1 )
    { printf("Dotted IP-address must be of the form a.b.c.d\n");
      exit(1);
    }
   /**********************************************************************/

   dest_addr.sin_port = atoi(argv[2]);           /* Destination (UDP) port # */

   /* ---
      Create a socket
      --- */
   s = socket(AF_INET, SOCK_DGRAM, 0);

   /* ---
      Set up socket end-point info for binding
      --- */
   in_addr.sin_family = AF_INET;                   /* Protocol domain */
   in_addr.sin_addr.s_addr = htonl(INADDR_ANY);    /* Use wildcard IP address */
   in_addr.sin_port = 0;	           	   /* Use any UDP port */

   /* ---
      Here goes the binding...
      --- */
   if (bind(s, (struct sockaddr *)&in_addr, sizeof(in_addr)) == -1)
    { printf("Error: bind FAILED\n");
    }
   else
    { printf("OK: bind SUCCESS\n");
    }

   /* ----
      Check what port I got
      ---- */
   len = sizeof(sock_addr);
   getsockname(s, (struct sockaddr *) &sock_addr, &len);
   printf("Socket s is bind to:\n");
   printf("  addr = %u\n", sock_addr.sin_addr.s_addr);
   printf("  port = %d\n", sock_addr.sin_port);

   while(1)
    { printf("Enter a line: ");
      scanf("%s", &line);

      /* ----
	 sendto() will automatically use UDP layer
	 ---- */
      sendto(s, line, strlen(line)+1, 0 /* flags */, 
	     (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    }
}
