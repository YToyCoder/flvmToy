#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
// tests for a stack vm

typedef long long Vm_int;
typedef double Vm_double;
typedef unsigned char Vm_char;
typedef bool Vm_bool;

typedef union {
  void *obj;
  Vm_int iv;
  Vm_double dv;
  Vm_char cv;
  Vm_bool bv;
} Value;

typedef Value* TStk;

class Frame{
  TStk stk_base;
  TStk stk_top;
  Value *locals;
  TStk stkp; // stack pointer
  size_t max_stk_size;
  size_t max_locals_size;

  public: 
    Frame(size_t max_stk_size, size_t max_locals_size){
      this->max_locals_size = max_locals_size;
      this->max_stk_size = max_stk_size;
    }

    void init(){
      stk_base = (TStk) malloc(sizeof(Value) * max_stk_size);
      stk_top = stk_base + max_stk_size;
      locals = (Value *) malloc(sizeof(Value) * max_locals_size);
    }

    void pushi(Vm_int v){
      stkp->bv = v;
      stkp++;
    }

};

int main(int argc, char const *argv[])
{
  return 0;
}
