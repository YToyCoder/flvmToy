#include <iostream>
#include <stdio.h>

class Declare;
typedef Declare *ThePointer;

int main(int argc, char const *argv[])
{
  printf("size of %d\n", sizeof(ThePointer));
  return 0;
}
