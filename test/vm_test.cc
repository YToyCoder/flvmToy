#include <iostream>
#include <stdio.h>
#include <stdint.h>
#include <string>

typedef long long Integer; // 8 byte
typedef double Double; // 8 byte

typedef union {
  Integer ivalue;
  Double dvalue;
} Value;

enum Instruction{
  ILoad,
  IADD,
  IStore
};

class TValue{
  public:
  Value val;
  uint8_t tag;
};

enum ValueTag {
  IntTag,
  DoubleTag
};

TValue createInt(Integer v){
  TValue ret;
  ret.val.ivalue = v;
  ret.tag = IntTag;
  return ret;
}

typedef Value *TStk; // stack element 8 byte

// instruction 8
// 4
// 20 : use 16
typedef uint32_t TOpcode;

uint8_t opcode2instr(TOpcode opcode){
  return (0xFF000000 & opcode) >> 24;
}


TOpcode sign_short(Integer val){
  val = (0x007FFFFF & val);
  if(val >> 31 == 1){
    return 0xFF800000 | val;
  }
  return val & 0xFF8FFFFF;
}

TOpcode encode2opcode(uint8_t instr, Integer payload){
  TOpcode ret = sign_short(payload);
  return  ret | (instr << 24);
}

Integer sign_extend(TOpcode opcode){
  Integer val = (0x007FFFFF & opcode);
  if(val >> 23 == 1){
    return (0xFFFFFFFFFFFFFFFF << 24) | val;
  }
  return val;
}


class Frame {
  public:
  TStk stk_base;
  TStk cursor;
  TStk stk_top;
  Value *locals;
  size_t local_size;
};

#define iload(frame, val) \
  (((frame)->cursor))->ivalue = (val); (frame)->cursor++ 

#define istore(frame, local, val) \
  (frame)->locals[local].ivalue = val

Integer popi(Frame *frame){
  frame->cursor--;
  Integer v = (frame->cursor)->ivalue;
  return v;
}

TOpcode encodingInstr(TOpcode code, uint8_t instr){
  return (instr << 24) | code;
}

TOpcode encodingstore(uint8_t instr, size_t local){
  return encodingInstr( 0 & (local << 20), instr);
}

uint8_t local1(TOpcode code){
  return (code & 0x00F00000) >> 20;
}

void iload_(Frame *frame, TOpcode code){
  Integer v = sign_extend(code);
  iload(frame,v);
}

void print_frame(Frame *frame){
  for(TStk i=frame->stk_base; i<=frame->stk_top; i++){
    printf("%d ",(i)->ivalue);
  }
  printf("\n");
  for(int i=0; i<frame->local_size; i++){
    printf("%d ",frame->locals[i].ivalue);
  }
  printf("\n");
}

void iadd(Frame *frame, TOpcode code){
  Integer v1 = popi(frame);
  Integer v2 = popi(frame);
  iload(frame, v1 + v2);
}

void istore_(Frame *frame, TOpcode code){
  Integer v_ = popi(frame);
  size_t local_ = local1(code);
  printf("store val(%d) to %d\n",v_, local_);
  istore(frame, local_, v_);
}

void dispatch(Frame *frame, TOpcode code){
    uint8_t instr = opcode2instr(code);
    switch(instr){
      case ILoad:
        iload_(frame, code);
        break;
      case IADD:
        iadd(frame, code);
        break;
      case IStore:
        istore_(frame, code);
        break;
    };
    print_frame(frame);
}

void init_stk(Frame *frame, size_t stk_size, size_t local_size){
  frame->stk_base = (TStk) malloc(sizeof(Value) * stk_size);
  frame->cursor = frame->stk_base;
  frame->stk_top = frame->stk_base + stk_size;
  frame->locals = (Value *) malloc(sizeof(Value) * local_size);
  frame->local_size = local_size;
}


void run(){
  size_t code_len = 4;
  TOpcode codes[code_len] = {
    encode2opcode(ILoad,10), 
    encode2opcode(ILoad, 11), 
    encode2opcode(IADD,0), 
    encodingstore(IStore,0)
  };
  Frame frame;
  init_stk(&frame, code_len, 1);
  for(size_t i=0; i<4;i++ ){
    TOpcode code = codes[i];
    printf("dispatch code %x\n", code);
    dispatch(&frame,code);
  }
}

void printOpAndInstr(TOpcode opcode){
  printf("opcode %x  instr %x", opcode, opcode2instr(opcode));
}

int main(int argc, char const *argv[])
{
  run();
  return 0;
}
