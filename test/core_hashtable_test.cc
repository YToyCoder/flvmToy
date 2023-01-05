#include "../flvmToy.hpp"
#include "../flvm_memory.cc"

class A {
  int a;
};

int main(int argc, char const *argv[])
{
  flvm::core::FlvmHashtable<A> t;
  return 0;
}
