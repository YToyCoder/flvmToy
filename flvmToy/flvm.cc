#include <iostream>
#include <unordered_map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <stdio.h>
#include <iomanip>
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


const time_t hash_seed = time(nullptr);

uint64_t hash_cstr(const char *p, size_t len){
  return MurmurHash64A(p, len, hash_seed);
}

// vm obj

std::ostream& operator<<(std::ostream& stream, const FlTagValue& value)
{
      switch(value.tag()){
        case FlTagValue::IntTag:    stream << value.union_v()._int; break;
        case FlTagValue::DoubleTag: stream << std::setprecision(10) <<(FlDouble) value.union_v()._double; break;
        case FlTagValue::ObjTag:    stream << "obj"; break;
        case FlTagValue::BoolTag:   stream << value.union_v()._bool; break;
        case FlTagValue::CharTag:   stream << value.union_v()._char; break;
        default:                    stream << "nil"; break;
      };

  return stream;
}
FlMethodBuilder::FlMethodBuilder()
{
  capability = 16;
  len = 0;
  max_stk = 0;
  max_locals = 0;
  code_cache = new instr_t[capability] ;// (instr_t*)malloc(sizeof(instr_t) * capability);
// const pool
  k_cap = 8;
  k_len = 0;
  k_cache = (FlTagValue*) malloc(sizeof(FlTagValue) * k_cap);
}
FlMethodBuilder::~FlMethodBuilder()
{
  if (nullptr != code_cache && NULL != code_cache)
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

FlMethod* FlMethodBuilder::build()
{
  FlMethod *ret = new FlMethod();
  ret->codes = new instr_t[len];
  for (int i = 0; i < len; i++) {
    ret->codes[i] = code_cache[i];
  }
  ret->_max_locals = max_locals;
  ret->_max_stk = max_stk;
  ret->_code_len = this->len;

  // consts
  ret->k = (FlTagValue *)malloc(sizeof(FlTagValue) * k_len);
  for (int i = 0; i < k_len; i++)
  {
    ret->k[i] = k_cache[i];
  }
  ret->_k_len = k_len;
  clear();
  return ret;
}

class FlStringConstPool 
{
  std::unordered_map<uint64_t,FlString *> pool;
  public:
  FlString *ofFlstring(const char *chars, size_t len){
    const uint64_t hash_code = hash_cstr(chars, len);
    FlString *obj = get(hash_code);
    if(obj == nullptr){
      obj = new FlString(const_cast<FlChar*>(chars), len);
      put(hash_code, obj);
    }
    return obj;
  }

  void put(uint64_t key, FlString * val){
    pool.emplace(key, val);
  }

  FlString *get(uint64_t key){
    auto iter = pool.find(key);
    if(iter == pool.end())
      return nullptr;
    return iter->second;
  }

};

class FlBinary {
};

class FlFileLoader {
  static uint8_t MagicNumber;
  public:
    void load(FlString *filename){
      std::ifstream file(filename->c_char(), std::ios::in | std::ios::binary);
      // magic number >> F1
      char mn;
      file.read(&mn, 1);
      // version
    }
};

inline std::string repeat(const std::string& str, size_t n){
  std::stringstream ret;
  for(int i=0; i<n; i++)
    ret << str;
  return ret.str();
}

inline std::string append_eq_util(const std::string& str, size_t n){
  return str + repeat(" ", n - str.size() - 1) + "=";
}

#define STR(content) #content
#define Case_Print(instruction)                                   \
  case Instruction::## instruction ##:                            \
  printf( STR( %03d  %04x < ## instruction>\n), ic_record, instr); \
  break;

#define Case_Print_One_Param(instruction)                                   \
  case Instruction::## instruction ##:                                      \
  printf( STR( %03d  %04x < ## instruction ## > %03d \n), ic_record, instr , codes[instr_cursor++]); \
  break;

void FlMethod::to_string() const
{
  size_t instr_cursor = 0;
  instr_t instr;
  size_t ic_record;
  while (instr_cursor < _code_len)
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
      printf("%03d %04x ipush %03d\n", ic_record, instr, sign_extend(codes[instr_cursor++]  << 8| codes[instr_cursor++]));
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
      printf("%03d %04x ldci %03d ", ic_record, instr, const_loc);
      std::cout << k[const_loc] << std::endl;
    }
      break;
    case Instruction::ldcd: 
    {
      uint8_t const_loc = codes[instr_cursor++];
      printf("%03d %04x ldcd %03d ", ic_record, instr, const_loc);
      std::cout << k[const_loc] << std::endl;
    }
      break;
    Case_Print(i2d)
    Case_Print(i2b)
    Case_Print(d2i)
    Case_Print_One_Param(istore)
    Case_Print_One_Param(dstore)
    Case_Print_One_Param(bstore)
    Case_Print_One_Param(ifeq)
    Case_Print_One_Param(iflt)
    Case_Print_One_Param(ifle)
    Case_Print_One_Param(ifgt)
    Case_Print_One_Param(ifge)
    Case_Print_One_Param(go)
    default:
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
void FlFrame::print_frame()
{
    printf("**********************frame data****************************\n");
    printf("** stk: ");
    for(FlTagValue *i=stk_base; i<stk_top; i++){
      if (i < stkp) std::cout << "[" << *i << "] ";
      else std::cout << *i << " ";
    }
    printf("\n** locals: ");
    for(int i=0; i<current_exec->max_locals(); i++)
        std::cout << locals[i] << " ";
    printf("\n** const: ");
    for (int i = 0; i < current_exec->_k_len; i++)
        std::cout << current_exec->k[i] << " ";
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
      _m_frame->loadd(_m_frame->popd(), read_instr());
      break;
    case Instruction::istore: 
      _m_frame->storei(_m_frame->popi(), read_instr()); 
      break;
    case Instruction::dstore: 
      _m_frame->stored(_m_frame->popd(), read_instr()); 
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
    case Instruction::i2d:
      _m_frame->pushd((FlDouble)_m_frame->popi());
      break;
    case Instruction::i2b:
      _m_frame->pushb((FlBool)_m_frame->popi());
      break;
    case Instruction::d2i:
      _m_frame->pushi(_m_frame->popd());
      break;

    default:
      throw std::exception(("not support instruction : " + std::to_string(instr)).c_str());
  };
#if 1
  printf("exec instr %x\n", instr);
  _m_frame->print_frame();
#endif
};

class FlExec {
  FlFrame *base_frame;
  FlFrame *current_frame;
  instr_t read_instr(){ return *(current_frame->pc++);}

  void _iconst_1(){ current_frame->pushi(1); }
  void _iconst_2(){ current_frame->pushi(2); }
  void _iconst_3(){ current_frame->pushi(3); }
  void _iconst_4(){ current_frame->pushi(4); }
  void _ipush()   { 
    FlInt v = sign_extend(read_instr() << 8 | read_instr());
    current_frame->pushi(v);
  }

  void _iload() { current_frame->loadi(current_frame->popi(), read_instr());}
  public:
    FlExec *setBase(FlFrame *frame){
      base_frame = frame;
      current_frame = base_frame;
      return this;
    }

    FlExec *setCurrent(FlFrame *frame){
      frame->setLast(current_frame);
      current_frame = frame;
      return this;
    }

    void dispatch(instr_t instr){
      switch(instr){
        case Instruction::iconst_1: _iconst_1(); break;
        case Instruction::iconst_2: _iconst_2(); break;
        case Instruction::iconst_3: _iconst_3(); break;
        case Instruction::iconst_4: _iconst_4(); break;
        case Instruction::dconst_1:
          current_frame->pushd(1.0);
          break;
        case Instruction::dconst_2:
          current_frame->pushd(2.0);
          break;
        case Instruction::ipush:    _ipush()   ; break;
        case Instruction::iload:    _iload()   ; break;
        case Instruction::dload:
          current_frame->loadd(current_frame->popd(), read_instr());
          break;
      };
#ifdef FlvmDebug
      current_frame->print_frame();
#endif
    }

    void run(){
      while(true){
        if(current_frame->end_of_pc()){
          printf("end of pc\n");
          return;
        }
        dispatch(*(current_frame->pc++));
      }
    }
};
