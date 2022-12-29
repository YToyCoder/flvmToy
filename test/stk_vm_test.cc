#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
// tests for a stack vm

typedef long long Vm_int;
typedef double Vm_double;
typedef unsigned char Vm_char;
typedef bool Vm_bool;
typedef uint8_t CodeUnit;

Vm_int sign_extend(uint16_t v){
  if((v >> 15) == 1){
    v |= 0xFFFFFFFFFFFF0000;
  }
  return v;
};

class FrameState;

class FuncProto {
  CodeUnit *codes;
  friend FrameState;
};

typedef union {
  void *obj;
  Vm_int iv;
  Vm_double dv;
  Vm_char cv;
  Vm_bool bv;
} Value;

typedef Value* TStk;

class Frame{
  protected:
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
      stkp->iv = v;
      stkp++;
    }

    void pushd(Vm_double v){
      stkp->dv = v;
      stkp++;
    }

    Vm_int popi(){
      stkp--;
      return stkp->iv;
    }

};


typedef enum {
  ipush = 0x01, // push 2byte as Vm_int
  iadd = 0x02, // add two top int value
} Instr;


class FrameState{
  Frame *frame;
  FuncProto *proto;
  FrameState *parent;
  public:
    typedef CodeUnit* TPc;
  private:
    TPc pc;

    Vm_int iread(){
      return sign_extend((*(pc++)) << 4 | *(pc++));
    }

    CodeUnit instr_read(){
      return *(pc++);
    }

};

class VmRuntimeContext {
  FrameState *main;
  FrameState *active_frame;
};

// void dispatch(Frame *frame, )

int main(int argc, char const *argv[])
{
  return 0;
}
