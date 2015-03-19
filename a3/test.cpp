#include "common.cpp"
#include <iostream>
#include <inttypes.h>
using namespace std;
int print(char* buf) {
  cout<<buf<< endl;
  return 0;
}
int main(int args , char **argv){
  getFile("test.cpp",print,10);
  return 0;
}
