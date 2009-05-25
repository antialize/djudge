#include <stdio.h>

int main() {
  while(true) {
    char * x = new char[1024*1024*10];
    for(int i=0; i < 1024*1024*10; ++i)
      x[i] = (13*i)%256;
  }    
}
