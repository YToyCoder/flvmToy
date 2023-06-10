#include <iostream>
#include <unordered_map>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
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
	code_cache = nullptr;
	capability = 16;
	len = 0;
	max_stk = -1;
	max_locals = -1;
	code_cache = (instr_t *) malloc(sizeof(instr_t) * capability);
}

void FlMethodBuilder::capability_check()
{
	if(len == capability){
		size_t extend = 1.5 * capability < INT32_MAX ? 1.5 * capability : INT32_MAX;
		instr_t* new_area = (instr_t *) malloc(sizeof(instr_t) * extend);
		for(int i=0; i<len; i++){
			new_area[i] = code_cache[i];
		}
		free(code_cache);
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

FlMethod* FlMethodBuilder::build()
{
	FlMethod *ret = new FlMethod();
	ret->codes = code_cache;
	ret->_max_locals = max_locals;
	ret->_max_stk = max_stk;
	ret->_code_len = this->len;
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
		size_t max_line_len = max(max(stks.size(), slocals.size()), head.size()) + 1;
		head += repeat("=", max_line_len - head.size());
		stks = append_eq_util(stks, max_line_len);
		slocals = append_eq_util(slocals, max_line_len);
		printf("%s\n",head.c_str());
		printf("%s\n",stks.c_str());
		printf("%s\n",slocals.c_str());
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
		default:
			throw std::exception(("not support instruction : " + std::to_string(instr)).c_str());
	};
#ifdef FlvmDebug
	current_frame->print_frame();
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