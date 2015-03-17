#include <sys/types.h>
#include <regex.h>
#include <iostream>
 
using namespace std; 
int main(int argc , char **argv) {
  //cout<<"Hello";  
  char lastbyte = (char) 0x132;
  for(int i = 0; i<15 ;i++){
    lastbyte = lastbyte ^ i ;
  }

  cout<<(int)lastbyte;
  return 0;
}
