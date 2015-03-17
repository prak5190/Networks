#include <inttypes.h>
/** Header struct , parsing and setting **/
struct udp_header {    
  uint16 ttl;  
  // // Total length 
  // uint64 length;

  // sequence and ack are of same length - TCP has 32 bit header so following it 
  // In reality even 16 bit is ok
  uint32 seq;  
  uint32 ack;
};
