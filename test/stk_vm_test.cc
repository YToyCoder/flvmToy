#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string>
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
  size_t code_size;
  friend FrameState;
  public:
    size_t c_size(){
      return code_size;
    }
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


class FrameInstrDisptcher {
  public:
  virtual void dispatch(CodeUnit instr) = 0;
  virtual FrameInstrDisptcher *renewFrame(FrameState *frame) = 0; 
};

class FrameState: public Frame{
  FuncProto *proto;
  FrameState *parent;
  public:
    typedef CodeUnit* TPc;
    FrameState(size_t max_ss, size_t max_ls, FuncProto *proto_): Frame(max_ss, max_ls) 
    {
      this->proto = proto_;
    }

    const char *toString(){
      std::string ret;
      for(TStk i=stk_base; i<stk_top; i++){
        ret += i->iv + " ";
      }
      ret += "\n";
      for(int i=0; i<max_locals_size; i++){
        ret += locals[i].iv + " ";
      }
      ret += "\n";
      return ret.c_str();
    }

    void setParent(FrameState *frame){
      parent = frame;
    }
  private:
    TPc pc;

  public:
    Vm_int iread(){
      return sign_extend((*(pc++)) << 8 | *(pc++));
    }

    CodeUnit instr_read(){
      return *(pc++);
    }

    bool stop(){
      return pc < (proto->codes + proto->c_size());
    }

    static FrameState *create(FuncProto *proto, size_t max_ss, size_t max_ls){
      // FrameState *ret = (FrameState *) malloc(sizeof(FrameState));
      return new FrameState(max_ss, max_ls, proto);
    }
};


class VmRuntimeContext {
  FrameState *main;
  FrameState *active_frame;
  FrameInstrDisptcher *instr_dispatcher;
  public:
    VmRuntimeContext(FrameInstrDisptcher *dispatcher){
      instr_dispatcher = dispatcher;
    }

    void push_frame(FrameState *frame){
      if(main == nullptr){
        main = frame;
        active_frame = frame;
      }else {
        frame->setParent(active_frame);
        active_frame = frame;
      }
      instr_dispatcher->renewFrame(active_frame);
    }

    void run(){
      active_frame->init();
      while(!active_frame->stop()){
        instr_dispatcher->dispatch(active_frame->instr_read());
      }
    }
};

class VirtualInstrDispatcher:public FrameInstrDisptcher {
  FrameState *frame;
  public:
  virtual VirtualInstrDispatcher *renewFrame(FrameState *frame) override 
  {
    frame = frame;
    return this;
  }

  void _ipush(){
    Vm_int v = frame->iread();
    frame->pushi(v);
  }

  void _iadd(){
    Vm_int v1 = frame->popi();
    Vm_int v2 = frame->popi();
    frame->pushi(v1 + v2);
  }

  virtual void dispatch(CodeUnit instr) override {
    switch(instr){
      case ipush:
        _ipush();
        break;
      case iadd:
        _iadd();
        break;
    };
    printf("%s\n", frame->toString());
  }
};


void run(){
  VirtualInstrDispatcher *dispatcher = new VirtualInstrDispatcher();
  VmRuntimeContext context(dispatcher);
  context.push_frame(FrameState::create(nullptr, 3, 1));
}

int main(int argc, char const *argv[])
{
  run();
  return 0;
}
