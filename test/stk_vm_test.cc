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
  static FuncProto *create(CodeUnit *vls,size_t len){
    FuncProto *ret = new FuncProto();
    ret->code_size = len;
    ret->codes = vls;
    printf("proto code start %x\n",vls);
    for(int i=0; i<len; i++){
      printf(" %x ", vls[i]);
    }
    printf("\n");
    return ret;
  }
};

typedef union Val{
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
      printf("call new Frame\n");
      this->max_locals_size = max_locals_size;
      this->max_stk_size = max_stk_size;
    }

    void init(){
      printf("call frame init\n");
      stk_base = (TStk) malloc(sizeof(Value) * max_stk_size);
      stk_top = stk_base + max_stk_size;
      for(int i=0; i<max_stk_size; i++){
        stk_base[i].iv = 0;
      }
      stkp = stk_base;
      locals = (Value *) malloc(sizeof(Value) * max_locals_size);
      for(int i=0; i<max_locals_size; i++){
        locals[i].iv = 0;
      }
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
  istore = 0x30, // store int to local variable
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
    FrameState(size_t max_ss, size_t max_ls, FuncProto *proto_): Frame{max_ss, max_ls} 
    {
      proto = proto_;
      pc = proto->codes;
      printf("created Frame start pc is %x\n", pc);
    }

    const char *toString(){
      std::string ret("frame content: ");
      ret += std::to_string(max_stk_size) + " ";
      ret += std::to_string(max_locals_size) + " | <stk> ";
      for(TStk i=stk_base; i<stk_top; i++){
        if(i >= stkp)
          ret += " " + std::to_string((*i).iv) + "  ";
        else 
          ret += "[" + std::to_string(i->iv)   + "] ";
      }
      ret += "| <locals> ";
      for(int i=0; i<max_locals_size; i++){
        ret += std::to_string(locals[i].iv) + " ";
      }
      ret += "\n";
      return ret.c_str();
    }

    void setParent(FrameState *frame){
      parent = frame;
    }
  protected:
    TPc pc;

  public:
    Vm_int iread(){
      return sign_extend((instr_read() << 8) | instr_read());
    }

    CodeUnit instr_read(){
      return *(pc++);
    }

    void storei(Vm_int v, size_t local){
      locals[local].iv = v;
    }

    bool stop(){
      return pc >= proto->codes + proto->c_size();
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
      main = nullptr;
      active_frame = nullptr;
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
      printf(active_frame->toString());
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
    this->frame = frame;
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

  void _istore(){
    frame->storei(frame->popi(), frame->instr_read());
  }

  virtual void dispatch(CodeUnit instr) override {
    switch(instr){
      case ipush:
        printf("dispatch ipush\n");
        _ipush();
        break;
      case iadd:
        printf("dispatch iadd\n");
        _iadd();
        break;
      case istore:
        printf("dispatch istore\n");
        _istore();
        break;
    };
    printf("%s\n", frame->toString());
  }
};

CodeUnit* int2code(int16_t vl_){
  CodeUnit * ret = (CodeUnit *) malloc(sizeof(CodeUnit) * 2);
  ret[0] = vl_ >> 8;
  ret[1] = vl_ & 0x00FF;
  return ret;
}


void run(){
  VirtualInstrDispatcher *dispatcher = new VirtualInstrDispatcher();
  VmRuntimeContext context(dispatcher);
  CodeUnit *av = int2code(10);
  CodeUnit *bv = int2code(11);
  CodeUnit *codes = (CodeUnit *) malloc(sizeof(CodeUnit) * 9);
  codes[0] = ipush;
  codes[1] = av[0];
  codes[2] = av[1];
  codes[3] = ipush;
  codes[4] = bv[0];
  codes[5] = bv[1];
  codes[6] = iadd;
  codes[7] = istore;
  codes[8] = 0;
  // {ipush, av[0], av[1], ipush, bv[0], bv[1], iadd}; 
  context.push_frame(FrameState::create(FuncProto::create(codes, 9) , 3, 1));
  context.run();
}

int main(int argc, char const *argv[])
{
  run();
  return 0;
}
