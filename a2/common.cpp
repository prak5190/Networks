#ifndef __COMMON__
#define __COMMON__ 1 

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdarg.h>

using namespace std; 

void error (const char* msg) {
  cout<<"Error "<<msg;
  exit(1);
}

// Include files you want to be commonly included - These files can be recurrsively required
#include "fileHandler.cpp"

#endif
