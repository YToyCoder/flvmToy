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


const time_t hash_seed = time(nullptr);

uint64_t hash_cstr(const char *p, size_t len){
  return MurmurHash64A(p, len, hash_seed);
}

// vm static define
FlStringConstPool *FlVM::const_string_pool = nullptr;
FlVM::State FlVM::_state = State::Uninit;

// vm obj
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
  k_cache = (FlTagValue*) malloc(sizeof(FlValue) * k_cap);
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
  ret->k = (FlTagValue *)malloc(sizeof(FlValue) * k_len);
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

inline std::string repeat(std::string str, size_t n){
  std::string ret;
  for(int i=0; i<n; i++)
    ret += str;
  return ret;
}

inline std::string append_eq_util(std::string str, size_t n){
  return str + repeat(" ", n - str.size() - 1) + "=";
}

std::string FlMethod::to_string() const
{
  std::stringstream ss;
  size_t instr_cursor = 0;
  instr_t instr;
  while (instr_cursor < _code_len)
  {
    instr = codes[instr_cursor++];
    switch (instr)
    {
		case Instruction::iconst_0: 
      printf("%04x iconst_0\n", instr);
      break;
		case Instruction::iconst_1: 
      printf("%04x iconst_1\n", instr);
      break;
		case Instruction::iconst_2: 
      printf("%04x iconst_2\n", instr);
      break;
		case Instruction::iconst_3: 
      printf("%04x iconst_3\n", instr);
      break;
		case Instruction::iconst_4: 
      printf("%04x iconst_4\n", instr);
      break;
		case Instruction::dconst_1:
      printf("%04x dconst_1\n", instr);
      break;
		case Instruction::dconst_2:
      printf("%04x dconst_2\n", instr);
      break;
		case Instruction::ipush:    
      printf("%04x ipush %03d\n", instr, codes[instr_cursor++]);
      break;
    case Instruction::dpush:
      printf("%04x dpush %03d\n", instr, codes[instr_cursor++]);
      break;
		case Instruction::iload:
      printf("%04x ipush %03d\n", instr, codes[instr_cursor++]);
      break;
		case Instruction::dload:
      printf("%04x dload %03d\n", instr, codes[instr_cursor++]);
      break;

		case Instruction::iadd: 
      printf("%04x iadd \n", instr);
      break;
		case Instruction::dadd: 
      printf("%04x dadd \n", instr);
      break;
		case Instruction::isub: 
      printf("%04x isub \n", instr);
      break;
		case Instruction::dsub: 
      printf("%04x dsub \n", instr);
      break;
		case Instruction::imul: 
      printf("%04x imul \n", instr);
      break;
		case Instruction::dmul: 
      printf("%04x dmul \n", instr);
      break;
		case Instruction::idiv: 
      printf("%04x idiv \n", instr);
      break;
		case Instruction::ddiv: 
      printf("%04x ddiv \n", instr);
      break;
    case Instruction::ldci: 
      printf("%04x idci %03d\n", instr, codes[instr_cursor++]);
      break;
    case Instruction::ldcd: 
      printf("%04x idcd %03d\n", instr, codes[instr_cursor++]);
      break;
    case Instruction::i2d:
      printf("%04x i2d \n", instr);
      break;
    case Instruction::i2b:
      printf("%04x i2b \n", instr);
      break;
    case Instruction::d2i:
      printf("%04x d2i \n", instr);
      break;
    default:
      throw std::exception("not support instruction when code to string");
    }
  }
  return ss.str();
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
		FlVM::error("stack out of index>>\n");
		exit(1);
	}
}

void FlFrame::print_frame()
{
		std::string head("==>>frame data<<");
		std::string stks("= stk: ");
		for(FlTagValue *i=stk_base; i<stk_top; i++){
			std::string el = i < stkp ? "[" + i->toString() + "]" : i->toString();
			stks += el + " ";
		}
		std::string slocals = "= locals: ";
		for(int i=0; i<current_exec->max_locals(); i++){
			slocals += locals[i].toString() + " ";
		}
    std::string s_konst = "= const: ";
    for (int i = 0; i < current_exec->_k_len; i++)
    {
      s_konst += current_exec->k[i].toString() + " ";
    }
		size_t max_line_len = max(max(max(stks.size(), slocals.size()), head.size()), s_konst.size()) + 1;
		head += repeat("=", max_line_len - head.size());
		stks = append_eq_util(stks, max_line_len);
		slocals = append_eq_util(slocals, max_line_len);
    s_konst = append_eq_util(s_konst, max_line_len);
		printf("%s\n",head.c_str());
		printf("%s\n",stks.c_str());
		printf("%s\n",slocals.c_str());
    printf("%s\n", s_konst.c_str());
		printf("%s\n",repeat("=",max_line_len).c_str());
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
// vm static
inline FlString *FlVM::ofString(const char *chars, size_t len){
  FlVM::state_check();
  return FlVM::const_string_pool->ofFlstring(chars, len);
}
inline void FlVM::init(){
#ifdef FlvmDebug
  printf("init\n");
#endif
  FlVM::_state = FlVM::State::Init;
  FlVM::const_string_pool = new FlStringConstPool();
}