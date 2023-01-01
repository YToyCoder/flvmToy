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
    return  v | 0xFFFFFFFFFFFF0000;
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

enum ObjKind {
  TInt,
  TDouble,
  TObj,
  TBool,
  UInit
};

#define valOfval(v, vt) ((v)._v.vt)
#define IntVal(v) (valOfval(v, iv))
#define DoubleVal(v) (valOfval(v, dv))
#define BoolVal(v) (valOfval((v), bv))
#define defineTValue(v) \
  valOfval(v, iv) = 0; \
  v.tag = UInit

class TValue{
  public:
  Value _v;
  uint8_t tag;
  TValue *setInt(Vm_int v){
    _v.iv = v;
    tag = TInt;
    return this;
  }

  TValue *setDouble(Vm_double v){
    _v.dv = v;
    tag = TDouble;
    return this;
  }

  TValue *setBool(Vm_bool v){
    BoolVal(*this) = v;
    tag = TBool;
    return this;
  }

  std::string toString(){
    switch(tag){
      case TInt:
        return std::to_string(_v.iv);
      case TDouble:
        return std::to_string(_v.dv);
      case TBool:
        return _v.bv ? "true" : "false";
      case UInit:
        return "<un>";
      default:
        return "<unreached>";
    }
  }
};

typedef TValue* TStk;

class Frame{
  protected:
  TStk stk_base;
  TStk stk_top;
  TValue *locals;
  TStk stkp; // stack pointer
  size_t max_stk_size;
  size_t max_locals_size;
  TValue _ret; // a return value

  public: 
    Frame(size_t max_stk_size, size_t max_locals_size){
      printf("call new Frame\n");
      this->max_locals_size = max_locals_size;
      this->max_stk_size = max_stk_size;
    }

    void init(){
      printf("call frame init\n");
      stk_base = (TStk) malloc(sizeof(TValue) * max_stk_size);
      stk_top = stk_base + max_stk_size;
      for(int i=0; i<max_stk_size; i++){
        // stk_base[i]._v.iv = 0;
        defineTValue(stk_base[i]);
      }
      stkp = stk_base;
      locals = (TValue *) malloc(sizeof(TValue) * max_locals_size);
      for(int i=0; i<max_locals_size; i++){
        defineTValue(locals[i]);
      }
    }

    void pushi(Vm_int v){
      stkp->setInt(v);
      stkp++;
    }

    void pushd(Vm_double v){
      stkp->setDouble(v);
      stkp++;
    }

    void pushb(Vm_bool v){
      stkp->setBool(v);
      stkp++;
    }

    TValue ret(){
      return _ret;
    }

    Vm_int popi(){
      stkp--;
      return stkp->_v.iv;
    }

    Vm_double popd(){
      stkp--;
      return stkp->_v.dv;
    }

    Vm_bool popb(){
      stkp--;
      return BoolVal(*stkp);
    }

};


typedef enum {
  ret = 0x01,
  ipush = 0x10, // push 2byte as Vm_int
  iadd = 0x11, // add two top int value
  istore = 0x12, // store int to local variable
  iload = 0x13, // local to stk top
  dpush = 0x21, // push 2byte as Vm_double
  dadd = 0x22, // add two top double value
  dload = 0x23, // local to stk top
  dstore = 0x23, // store double to local variable
  bpusht = 0x30, // true to stk
  bpushf = 0x31, // false to stk
  bstore = 0x32, // bool to local
} Instr;

enum BoolInst{
  FalseIns = 0, // false
  TrueIns = 1 // true
};

class FrameInstrDisptcher {
  public:
  virtual void dispatch(CodeUnit instr) = 0;
  virtual FrameInstrDisptcher *renewFrame(FrameState *frame) = 0; 
};

class FrameState: public Frame{
  FuncProto *proto;
  FrameState *parent;
  bool _stop = false;
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
          ret += " " + i->toString() + "  ";
        else 
          ret += "[" + i->toString()  + "] ";
      }
      ret += "| <locals> ";
      for(int i=0; i<max_locals_size; i++){
        ret += locals[i].toString() + " ";
      }
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

    Vm_double dread(){
      Vm_double ret;
      char *temp=(char *)&ret;
      for(int i=0; i<8; i++){
        *temp = instr_read();
        temp++;
      }
      return ret;
    }

    CodeUnit instr_read(){
      return *(pc++);
    }

    void storei(Vm_int v, size_t local){
      locals[local].setInt(v);
    }

    void stored(Vm_double v, size_t local){
      locals[local].setDouble(v);
    }

    void storeb(Vm_bool v, size_t local){
      locals[local].setBool(v);
    }

    TValue localVar(uint8_t local){
      return locals[local];
    }

    void stop(){
      _stop = true;
    }

    bool stoped(){
      return _stop;
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
      while(!active_frame->stoped()){
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

  void _dpush(){
    frame->pushd(frame->dread());
  }

  void _dadd(){
    Vm_double v1 = frame->popd();
    Vm_double v2 = frame->popd();
    frame->pushd(v1 + v2);
  }

  void _dstore(){
    frame->stored(frame->popd(), frame->instr_read());
  }

  void _bpush(uint8_t v){
    switch(v){
      case FalseIns:
      case TrueIns:
        frame->pushb(v);
        break;
      default:
        //error
        printf("error instance of bool \n");
    }
  }

  void _bstore(){
    frame->storeb(frame->popb(), frame->instr_read());
  }

  void _iload(){
    Vm_int v = IntVal(frame->localVar(frame->instr_read()));
    frame->pushi(v);
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
      case iload:
        printf("dispatch iload\n");
        _iload();
        break;
      case dpush:
        printf("dispatch dpush\n");
        _dpush();
        break;
      case dstore:
        printf("dispatch dstore\n");
        _dstore();
        break;
      case dadd:
        printf("dispatch dadd\n");
        _dadd();
        break;
      case ret:
        printf("dispatch ret\n");
        frame->stop();
        break;
      case bpusht:
        printf("dispatch bpusht\n");
        _bpush(true);
        break;
      case bpushf:
        printf("dispatch bpushf\n");
        _bpush(false);
        break;
      case bstore:
        printf("dispatch bstore\n");
        _bstore();
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

class CodeBuilder{
  CodeUnit *cache;
  size_t capability;
  size_t pointer;
  public:
    CodeBuilder *push(CodeUnit *carr,size_t len){
      check_capability(pointer + len);
      for(size_t i=0; i<len; i++){
        cache[pointer + i] = carr[i];
      }
      pointer += len;
      return this;
    }

    CodeUnit *codes(){
      return cache;
    }

    CodeBuilder(size_t _size = 16){
      this->capability = _size;
      this->cache = (CodeUnit *) malloc(sizeof(CodeUnit) * _size);
      this->pointer = 0;
    }

    CodeBuilder *append2biteInt(int16_t _vl){
      CodeUnit *temp = int2code(_vl);
      push(temp, 2);
      free(temp);
      return this;
    }

    CodeBuilder *append8bitDouble(double _vl){
      return push((CodeUnit *)&_vl, 8);
    }

    size_t size(){
      return pointer;
    }

    CodeBuilder *append(CodeUnit code){
      return push(&code, 1);
    }

    ~CodeBuilder(){
      free(cache);
    }

  private:

    void check_capability(size_t expect_len){
      if(expect_len > capability){
        capability_extends(capability * 1.5 < expect_len ? expect_len : capability * 1.5);
      }
    }
    void capability_extends(size_t extend_len){
      if(extend_len > capability){
        CodeUnit *stotarage = (CodeUnit *) malloc((sizeof (CodeUnit)) * extend_len);
        for(size_t i=0; i<pointer; i++){
          stotarage[i] = cache[i];
        }
        free(cache);
        cache = stotarage;
      }
    }

};


void run(){
  VirtualInstrDispatcher *dispatcher = new VirtualInstrDispatcher();
  VmRuntimeContext context(dispatcher);
  CodeUnit *av = int2code(10);
  CodeUnit *bv = int2code(11);
  CodeUnit *codes = (CodeUnit *) malloc(sizeof(CodeUnit) * 9);
  // {ipush, av[0], av[1], ipush, bv[0], bv[1], iadd}; 
  CodeBuilder builder(6);
  builder.append(ipush);
  builder.append2biteInt(-1);
  builder.append(ipush);
  builder.append2biteInt(11);
  builder.append(iadd);
  builder.append(istore);
  builder.append(0);
  builder.append(iload);
  builder.append(0);
  builder.append(dpush);
  builder.append8bitDouble(1.3);
  builder.append(dpush);
  builder.append8bitDouble(1.0);
  builder.append(dadd);
  builder.append(dstore);
  builder.append(1);
  builder.append(bpushf);
  builder.append(bpusht);
  builder.append(bstore);
  builder.append(0);
  builder.append(ret);
  context.push_frame(FrameState::create(FuncProto::create(builder.codes(), builder.size()) , 5, 2));
  context.run();
}

// CodeUnit* double2code()

int main(int argc, char const *argv[])
{
  run();
  // printf("float len %d\n", sizeof(float));
  // printf("sign_extends double\n");
  return 0;
}
