#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

using namespace std; 

struct stackel {
  // Storing index
  int val;
  stackel *next;
};
struct stack {
  stackel *head;
  void push(int pos) {
    stackel* temp = new stackel();
    temp->val = pos ; 
    temp->next = this->head;
    this->head = temp;
  };
  int pop() {
    int ret = -1;
    stackel* temp;
    if (head != NULL) {      
      ret = this->head->val;
      temp = head->next;
      delete head;
    };
    head = temp;
    return ret;
  };
  void empty() {
    while( this->pop()!=-1 );
  };
};

// Parse string and return true false and position if false
void parseUsingLL(const char* str) {
  const char *k = str ;
  int i = 0;
  stack *st = new stack();
  while (*str != '\0') {    
    char c = *str;
    // if ")" pop the stack 
    if (c == '(') {
      st->push(i);
    } else if (c == ')') {
      // if "(" push into stack the position    
      if(st->pop() == -1) {
        cout<<"False "<<i+1<<endl;
        return;
      }
    }
    // when string finishes or stack is empty - then 
    str++;i++;
  };
  // If stack is not empty - then print position of rightmost paren
  int b = st->pop();
  if(b != -1) {
    cout<<"False "<<b+1<<endl;
  } else {
    cout<<"True"<<endl;
  }
    
  // Delete the stack
  st->empty();
  delete st;
}

void parseUsingArray(char arr[],int len) {
  int stack[len];
  int stackPos = -1;
  for (int i=0; i <len; i++) {
    char c = arr[i];
    if (c == '(') {
      // Push position into an array
      stackPos++;
      //      cout<<"Storing into stack "<<i<<"  "<<stackPos<<endl;
      stack[stackPos] = i;
    } else if (c == ')') {
      //      cout<<"popping from stack "<<stackPos <<"  " << stack[stackPos]<<endl;
      stackPos--;
      if (stackPos < -1) {
        cout<<"False "<<i+1<<endl;
        return;
      } 
    }
  }

  if (stackPos == -1) {
    cout<<"True"<<endl;
  } else {
    // cout<<stackPos;
    // for(int i=0;i<=stackPos;i++)
    //   cout<<endl<<stack[stackPos];
    cout<<"False "<<stack[stackPos]+1<<endl;
  }
}

int main (int argc ,char** argv) {
  int arrLen = 0 ;
  char* arr;
  cout<<endl;
  // Assumes type if text
  int itype = 0, ptype = 0;
  // Need to parse arguments 
  int i ;
  for (i=0; i < argc-1 ; i++) {
    //    cout<<endl<<argv[i];
    // Parse program arguments 
    if (strcmp(argv[i],"--array") == 0) {
      if (i < argc-1) {        
        // Get next argument
        arrLen = atoi(argv[i+1]);      
        i++;
        ptype = 0;
      } else {
        cout<<"Pass length of array after --array like --array 100";
        return -1;
      }
    } else if (strcmp(argv[i],"--link") == 0) {
      ptype = 1;
    } else if (strcmp(argv[i],"--text") == 0) {
      // Last argument is text
      // text is 0 -- TODO convert to enum
      itype = 0;
    } else if (strcmp(argv[i],"--file") == 0) {
      //      cout<<"filllleeeeeeeeeeeeeeE";
      // File is 1
      itype = 1;
    }
  };
  // Get final argument according to type and proceed 
  if (itype == 0) {
    char *t = argv[i];
    // Get the text 
    if (ptype == 0) {
      parseUsingArray(t,arrLen);
    } else {
      parseUsingLL(t);
    }
  } else {
    cout<<"Using file : "<<argv[i]<<endl;
    FILE* fd = NULL ;   
    fd = fopen(argv[i],"r");
    if(NULL == fd)
    {
      cout<<"Unable to open file"<<endl;
      return -1;
    } else {
       char * line = NULL;
       size_t len = 0;
       while(getline(&line, &len, fd) != -1) {
         //cout<<line<<endl;
         // Process the file i.e get a line of the file and accordingly pass it 
         if (ptype == 0) {
           parseUsingArray(line,arrLen);
           line = NULL;
         } else {      
           parseUsingLL(line);
         }         
       };
    }
  }
  cout<<endl;
  return 0;
}
