#pragma once
#include "flvm.hpp"
#include "Fn.h"

class ExecFrame {
public:
  void pushd(FlDouble v){ stkp->set(v); stkp++; }
  void pushb(FlBool v)  { stkp->set(v); stkp++; }
  void pushi(FlInt v)   { stkp->set(v); stkp++; }
  template<typename _Ty>
  void pushv(_Ty v) { stkp->set(v); stkp++; }
  inline FlInt    read_locali(size_t loc) {  return locals[loc]._int(); }
  inline FlDouble read_locald(size_t loc) {  return locals[loc]._double(); }
public:
  ExecFrame* next;
  ExecFrame* pre;

  FnProto* proto;
  instr_t* pc; // pointer to code
  FlTagValue* locals; 
  FlTagValue* k;

  FlTagValue* stkb; // base
  FlTagValue* stkt; // top
  FlTagValue* stkp; // pointer
};

// emulating a virtual machine
class Emulator {
private:
  void dispatch_instruction(instr_t instr) ;
// frame operations
  inline instr_t read_instr(){ return *(topFrame->pc++);}
  inline FlInt    popi() { return (topFrame->stkp--)->_int(); }
  inline FlDouble popd() { return (topFrame->stkp--)->_double(); }
  inline FlBool   popb() { return (topFrame->stkp--)->_bool(); }
  inline FlString* pops() { return (topFrame->stkp--)->_str(); }
  inline FlTagValue ldconst(instr_t loc) { return topFrame->k[loc]; }
  inline void set_pc(uint16_t offset) { topFrame->pc = topFrame->proto->code + offset; }

// instruction impl
  inline void iconst_0() { topFrame->pushi(0); }
  inline void iconst_1() { topFrame->pushi(1); }
  inline void iconst_2() { topFrame->pushi(2); }
  inline void iconst_3() { topFrame->pushi(3); }
  inline void iconst_4() { topFrame->pushi(4); }
  inline void dconst_1() { topFrame->pushd(1.0); }
  inline void dconst_2() { topFrame->pushd(2.0); }
  inline void pushi() { 
    FlInt intValue = sign_extend(read_instr() | read_instr() << 8);
    topFrame->pushi(intValue);
  }
  inline void pushd() { topFrame->pushd(read_instr()); }
  inline void loadi() { topFrame->pushi(topFrame->read_locali(read_instr())); }
  inline void loadd() { topFrame->pushd(topFrame->read_locald(read_instr())); }
  inline void storei(){ topFrame->locals[read_instr()].set(popi()); }
  inline void stored(){ topFrame->locals[read_instr()].set(popd()); }
  inline void storeb(){ topFrame->locals[read_instr()].set(popb()); }
  inline void stores(){ topFrame->locals[read_instr()].set(pops()); }
  inline void addi()  { topFrame->pushi(popi() + popi()); }
  inline void addd()  { topFrame->pushd(popd() + popd()); }
  inline void subi()  { 
    FlInt topI = popi();
    topFrame->pushi(popi() - topI); 
  }
  inline void subd()  { 
    FlDouble topD = popd();
    topFrame->pushd(popd() - topD); 
  }
  inline void muli()  { topFrame->pushi(popi() * popi()); }
  inline void muld()  { topFrame->pushd(popd() * popd()); }
  inline void divi()  { 
    FlInt topI = popi();
    topFrame->pushi(popi() / topI); 
  }
  inline void divd()  { 
    FlDouble topD = popd();
    topFrame->pushd(popd() / topD); 
  }

  inline void ldci() { topFrame->pushv(ldconst(read_instr())._int()); }
  inline void ldcd() { topFrame->pushv(ldconst(read_instr())._double()); }
  inline void ldcs() { topFrame->pushv(ldconst(read_instr())._str()); }
  inline void i2d()  { topFrame->pushv((FlDouble)popi()); }
  inline void i2b()  { topFrame->pushv((FlBool)popi()); }
  inline void d2i()  { topFrame->pushv((FlInt)popd()); }
private:
  ExecFrame* topFrame;
};
