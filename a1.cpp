#include <iostream>
#include <cstdio>

using namespace std; 

struct stackel {
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
const char* parseUsingArray(const char* str) {
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
        cout<<"False "<<i;
        break;
      }
    }
    // when string finishes or stack is empty - then 
    str++;i++;
  };
  // If stack is not empty - then print position of rightmost paren
  int b = st->pop();
  if(b != -1) {
    cout<<"False "<<b;
  } else {
    cout<<"True";
  }
    
  // Delete the stack
  st->empty();
  delete st;
}

int main (int argc ,char** argv) {
  parseUsingArray("()(");
  return 0;
}
