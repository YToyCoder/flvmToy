#include <iostream>
#include <unordered_map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <stdio.h>
#include "flvm.hpp"
#include "MurmurHash2.h"

// color print
void COLOR_PRINT(const char* s, int color)
{
#if defined(WIN32) || defined(_WIN32)
  HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
  SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | color);
  printf(s);
  SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | 7);
#else 
  printf(s);
#endif
}

// ****************** instruction map *******************
const char * instr_map[256];

const char* instr_name(Instruction::Code instr)
{
  return instr_map[instr];
}

void set_instr_map()
{
  InstrNameDefine(nop);
  InstrNameDefine(iconst_0);
  InstrNameDefine(iconst_1);
  InstrNameDefine(iconst_2);
  InstrNameDefine(iconst_3);
  InstrNameDefine(iconst_4);
  InstrNameDefine(dconst_1);
  InstrNameDefine(dconst_2);
  InstrNameDefine(ipush);
  InstrNameDefine(dpush);
  InstrNameDefine(iload);
  InstrNameDefine(dload);
  InstrNameDefine(aload);
  InstrNameDefine(istore);
  InstrNameDefine(dstore);
  InstrNameDefine(bstore);
  InstrNameDefine(ostore);
  InstrNameDefine(ldci);
  InstrNameDefine(ldcd);
  InstrNameDefine(ldcs);
  InstrNameDefine(iadd);
  InstrNameDefine(dadd);
  InstrNameDefine(isub);
  InstrNameDefine(dsub);
  InstrNameDefine(imul);
  InstrNameDefine(dmul);
  InstrNameDefine(idiv);
  InstrNameDefine(ddiv);
  InstrNameDefine(ifeq);
  InstrNameDefine(iflt);
  InstrNameDefine(ifle);
  InstrNameDefine(ifgt);
  InstrNameDefine(ifge);
  InstrNameDefine(go);
  InstrNameDefine(dcmp);
  InstrNameDefine(i2d);
  InstrNameDefine(i2b);
  InstrNameDefine(d2i);
}


const time_t hash_seed = time(nullptr);

uint64_t hash_cstr(const char *p, size_t len){
  return MurmurHash64A(p, len, hash_seed);
}

FlMethodBuilder::FlMethodBuilder()
{
  capability = 16;
  len = 0;
  max_stk = 0;
  max_locals = 0;
  code_cache = new instr_t[capability] ;

// const pool
  k_cap = 8;
  k_len = 0;
  k_cache = (FlTagValue*) malloc(sizeof(FlTagValue) * k_cap);
}
FlMethodBuilder::~FlMethodBuilder()
{
  if (nullptr != code_cache)
  {
    delete[] code_cache;
  }
  if (nullptr != k_cache)
  {
    free(k_cache);
  }
}

void FlMethodBuilder::capability_check()
{
  if(len == capability){
    size_t extend = 1.5 * capability < INT32_MAX ? 1.5 * capability : INT32_MAX;
    instr_t* new_area =new instr_t[extend] ; 
    if (new_area == NULL)
    {
      throw std::runtime_error("alloc memory failed when build method");
    }
    for(int i=0; i<len && i<extend; i++){
      new_area[i] = code_cache[i];
    }
    delete[] code_cache;
    capability = extend;
    code_cache = new_area;
  }
}

FlMethodBuilder* FlMethodBuilder::set_max_stk(size_t stk_deep)
{
  max_stk = stk_deep;
  return this;
}

FlMethodBuilder* FlMethodBuilder::set_max_locals(size_t locals_size)
{
  max_locals = locals_size;
  return this;
}

FlMethodBuilder* FlMethodBuilder::append(instr_t instr)
{
  capability_check();
  code_cache[len++] = instr;
  return this;
}

uint8_t FlMethodBuilder::store_const_int(FlInt _i)
{
  const_pool_size_check();
  uint8_t const_loc = k_len;
  k_cache[k_len++].set( _i);
  return const_loc;
}

uint8_t FlMethodBuilder::store_const_double(FlDouble _d)
{
  const_pool_size_check();
  uint8_t const_loc = k_len;
  k_cache[k_len++].set(_d);
  return const_loc;
}

uint8_t FlMethodBuilder::store_const_str(FlString* str)
{
  const_pool_size_check();
  uint8_t const_loc = k_len;
  k_cache[k_len++].set(str);
  return const_loc;
}

FlMethod* FlMethodBuilder::build()
{
  FlMethod *ret = new FlMethod();
  ret->codes = new instr_t[len];
  for (int i = 0; i < len; i++) {
    ret->codes[i] = code_cache[i];
  }
  ret->m_max_locals = max_locals;
  ret->m_max_stk = max_stk;
  ret->code_len = this->len;

  // consts
  ret->k = (FlTagValue *)malloc(sizeof(FlTagValue) * k_len);
  for (int i = 0; i < k_len; i++)
  {
    ret->k[i] = k_cache[i];
  }
  ret->k_len = k_len;
  clear();
  return ret;
}

#define STR(content) #content
#define Case_Print(instruction)                                     \
  case Instruction::## instruction ##:                              \
  printf( STR( %03d  %04x %10s\n), ic_record, instr, #instruction); \
  break;

#define Case_Print_One_Param(instruction)                                   \
  case Instruction::## instruction ##:                                      \
  printf( STR( %03d  %04x %10s %03d \n), ic_record, instr , #instruction , codes[instr_cursor++]); \
  break;

void FlMethod::to_string() const
{
  size_t instr_cursor = 0;
  instr_t instr;
  int ic_record;
  for(int i=0; i < k_len; i++) {
    printf("%02d ", i);
    std::cout << k[i] << std::endl; 
  }
  while (instr_cursor < code_len)
  {
    ic_record = instr_cursor;
    instr = codes[instr_cursor++];
    switch (instr)
    {
    Case_Print(iconst_0)
    Case_Print(iconst_1)
    Case_Print(iconst_2)
    Case_Print(iconst_3)
    Case_Print(iconst_4)
    Case_Print(dconst_1)
    Case_Print(dconst_2)
    case Instruction::ipush: 
    {
      uint8_t i1 = codes[instr_cursor++];
      uint8_t i2 = codes[instr_cursor++];
      printf("%03d %04x %10s %03d\n", ic_record, instr, "ipush", sign_extend(i1 | ( i2 << 8) ));
    }
      break;
    Case_Print_One_Param(dpush)
    Case_Print_One_Param(iload)
    Case_Print_One_Param(dload)

    Case_Print(iadd)
    Case_Print(dadd)
    Case_Print(isub)
    Case_Print(dsub)
    Case_Print(imul)
    Case_Print(dmul)
    Case_Print(idiv)
    Case_Print(ddiv)
    case Instruction::ldci: 
    {
      uint8_t const_loc = codes[instr_cursor++];
      printf("%03d %04x %10s #%03d ", ic_record, instr, "ldci", const_loc);
      std::cout << k[const_loc] << std::endl;
    }
      break;
    case Instruction::ldcd: 
    {
      uint8_t const_loc = codes[instr_cursor++];
      printf("%03d %04x %10s #%03d ", ic_record, instr, "ldcd", const_loc);
      std::cout << k[const_loc] << std::endl;
    }
      break;
    case Instruction::ldcs: 
    {
      uint8_t const_loc = codes[instr_cursor++];
      printf("%03d %04x %10s #%03d ", ic_record, instr, "ldcd", const_loc);
      std::cout << k[const_loc] << std::endl;
    }
      break;

    Case_Print(i2d)
    Case_Print(i2b)
    Case_Print(d2i)
    Case_Print(dcmp)
    Case_Print_One_Param(istore)
    Case_Print_One_Param(dstore)
    Case_Print_One_Param(bstore)
    Case_Print_One_Param(ostore)
    Case_Print_One_Param(ifeq)
    Case_Print_One_Param(iflt)
    Case_Print_One_Param(ifle)
    Case_Print_One_Param(ifgt)
    Case_Print_One_Param(ifge)
    Case_Print_One_Param(go)
    default:
      printf("not support instruction (%04x:%s) to string\n", instr, instr_name((Instruction::Code)instr));
      throw std::exception("not support instruction when code to string");
    }
  }
}

void FlFrame::init()
{
  // pc
  pc = current_exec->codes;
  // locals
  const size_t max_locals = current_exec->max_locals();
  locals = (FlTagValue *) malloc(sizeof(FlTagValue) * max_locals);
  for(int i=0; i< max_locals; i++){
    locals[i]._tag = FlTagValue::UnInit;
  }
  // stk
  const size_t max_stk = current_exec->max_stk();
  stk_base = (FlTagValue *) malloc(sizeof(FlTagValue) * max_stk);
  stk_top = stk_base + max_stk;
  stkp = stk_base;
  for(FlTagValue *i=stk_base; i<stk_top; i++){
    i->_tag = FlTagValue::UnInit;
  }
}

void FlFrame::stkp_out_of_index_check()
{
  if(stkp < stk_base || stkp >= stk_top){
    exit(1);
  }
}

void append(std::stringstream& stream, size_t max_len)
{
  size_t append_size = max_len - stream.gcount();
  for (int i = 0; i < append_size; i++)
  {
    stream << " ";
  }
  stream << "=";
}
//#define CPrint
void FlFrame::print_frame(int32_t instr)
{
    printf("**********************frame data****************************\n");
    printf("** instr: %03x %s\n", instr, instr_name((Instruction::Code)instr));
    printf("** stk: ");
    for(FlTagValue *i=stk_base; i<stk_top; i++){
      if (i + 1 == stkp) std::cout << "<[*" << *i << "]> ";
      else if (i < stkp) std::cout << "<[" << *i << "]> ";
      else std::cout << "[" << *i << "] ";
    }
    printf("\n** locals: ");
    for(int i=0; i<current_exec->max_locals(); i++)
        std::cout << locals[i] << " ";
    printf("\n** const: ");
    for (int i = 0; i < current_exec->k_len; i++)
        std::cout << "[" << current_exec->k[i] << "] ";
    printf("\n************************************************************\n");
}


FlInt sign_extend(uint16_t v){
  if((v >> 15) == 1){
    return  v | 0xFFFFFFFFFFFF0000;
  }
  return v;
};

void FlSExec::dispatch(instr_t instr)
{
  switch(instr){
    case Instruction::nop:                   break;
    case Instruction::iconst_0: _iconst_0(); break;
    case Instruction::iconst_1: _iconst_1(); break;
    case Instruction::iconst_2: _iconst_2(); break;
    case Instruction::iconst_3: _iconst_3(); break;
    case Instruction::iconst_4: _iconst_4(); break;
    case Instruction::dconst_1:
      _m_frame->pushd(1.0);
      break;
    case Instruction::dconst_2:
      _m_frame->pushd(2.0);
      break;
    case Instruction::ipush:    _ipush()   ; break;
    case Instruction::dpush:    _dpush()   ; break;
    case Instruction::iload:    _iload()   ; break;
    case Instruction::dload:
      _m_frame->loadd(read_instr());
      break;
    case Instruction::istore: 
      _m_frame->storei(_m_frame->popi(), read_instr()); 
      break;
    case Instruction::dstore: 
      _m_frame->stored(_m_frame->popd(), read_instr()); 
      break;
    case Instruction::bstore: 
      _m_frame->storeb(_m_frame->popb(), read_instr()); 
      break;
    case Instruction::sstore: 
      _m_frame->stores(_m_frame->pops(), read_instr()); 
      break;

    case Instruction::iadd: _m_frame->iadd(); break;
    case Instruction::dadd: _m_frame->dadd(); break;
    case Instruction::isub: _m_frame->isub(); break;
    case Instruction::dsub: _m_frame->dsub(); break;
    case Instruction::imul: _m_frame->imul(); break;
    case Instruction::dmul: _m_frame->dmul(); break;
    case Instruction::idiv: _m_frame->idiv(); break;
    case Instruction::ddiv: _m_frame->ddiv(); break;
    case Instruction::ldci: _m_frame->ldci(read_instr()); break;
    case Instruction::ldcd: _m_frame->ldcd(read_instr()); break;
    case Instruction::ldcs: _m_frame->ldcs(read_instr()); break;
    case Instruction::i2d:
      _m_frame->pushd((FlDouble)_m_frame->popi());
      break;
    case Instruction::i2b:
      _m_frame->pushb((FlBool)_m_frame->popi());
      break;
    case Instruction::d2i:
      _m_frame->pusho(_m_frame->popo());
      break;

#define Case_Comp(instruction, op) \
    case Instruction:: ##instruction : {              \
      FlInt top_int = _m_frame->popi();               \
      uint8_t jump_offset = read_instr();             \
      if(top_int op 0) _m_frame->set_pc(jump_offset); \
    }                                                 \
    break;

    Case_Comp(ifeq, ==)
    Case_Comp(iflt, <)
    Case_Comp(ifle, <=)
    Case_Comp(ifgt, >)
    Case_Comp(ifge, >=)
    case Instruction::go: _m_frame->set_pc(read_instr()); break;
    case Instruction::dcmp:
      {
        FlDouble d1 = _m_frame->popd();
        FlDouble d2 = _m_frame->popd();
        FlDouble sub = d2 - d1;
        if(sub == 0) _m_frame->pushi(0);
        if(sub < 0) _m_frame->pushi(-1);
        if(sub > 0) _m_frame->pushi(1);
      }
      break;

    default:
      printf("not support instruction : %s \n", instr_name((Instruction::Code)instr));
      throw std::exception(("not support instruction : " + std::to_string(instr)).c_str());
  };
#if 1
  _m_frame->print_frame(instr);
#endif
};

void Emulator::dispatch_instruction(instr_t instr) {
  switch(instr) {
    case Instruction::nop:                  break;
    case Instruction::iconst_0: iconst_0(); break;
    case Instruction::iconst_1: iconst_1(); break;
    case Instruction::iconst_2: iconst_2(); break;
    case Instruction::iconst_3: iconst_3(); break;
    case Instruction::iconst_4: iconst_4(); break;
    case Instruction::dconst_1: dconst_1(); break;
    case Instruction::dconst_2: dconst_2(); break;
    case Instruction::ipush:    pushi();    break;
    case Instruction::dpush:    pushd();    break;
    case Instruction::iload:    loadi();    break;
    case Instruction::dload:    loadd();    break;
    case Instruction::istore:   storei();   break;
    case Instruction::dstore:   stored();   break;
    case Instruction::bstore:   storeb();   break;
    case Instruction::sstore:   stores();   break;
    case Instruction::iadd:     addi();     break;
    case Instruction::dadd:     addd();     break;
    case Instruction::dsub:     subd();     break;
    case Instruction::isub:     subi();     break;
    case Instruction::imul:     muli();     break;
    case Instruction::dmul:     muld();     break;
    case Instruction::idiv:     divi();     break;
    case Instruction::ddiv:     divd();     break;
    case Instruction::ldci:     ldci();     break;
    case Instruction::ldcd:     ldcd();     break;
  };
}