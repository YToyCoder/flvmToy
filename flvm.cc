#include <iostream>
#include <unordered_map>
#include <string>
#include "MurmurHash2.cc"
#include <iostream>
#include <sstream>
#include <fstream>

#if defined(WIN32) || defined(_WIN32)
#include <windows.h>
struct TermColor {
  enum {
  Black = 0,
  Green = 2,
  Red   = 4,
  };
};
#endif

// color print
void COLOR_PRINT(const char* s, int color)
{
#if defined(WIN32) || defined(_WIN32)
  HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
  SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | color);
  printf(s);
  SetConsoleTextAttribute(handle, FOREGROUND_INTENSITY | 7);
#endif
}


const time_t hash_seed = time(nullptr);

uint64_t hash_cstr(const char *p, size_t len){
  return MurmurHash64A(p, len, hash_seed);
}
struct Instruction {
  enum Code {
    iconst_1 = 0x01,
    iconst_2 = 0x02,
    iconst_3 = 0x03,
    iconst_4 = 0x04,
    dconst_1 = 0x05,
    dconst_2 = 0x06,
    ipush    = 0x07,
    iload    = 0x08,
    dload    = 0x09,
    aload    = 0x0A,
    ldci     = 0x11,
    ldcd     = 0x12,
  };
};

// instruction type : 1 byte
typedef uint8_t instr_t;
// vm types

class FlObj {
  virtual uint64_t hash() = 0;
};

typedef long long FlInt;
typedef double    FlDouble;
typedef bool      FlBool;
typedef char      FlChar;
typedef uint8_t   FlByte;
typedef FlObj*    FlObjp;

// vm slot
union FlValue {
  FlInt _int;
  FlDouble _double;
  FlObjp _obj;
  FlBool _bool;
  FlChar _char;
  FlByte _byte;
};

typedef union FlValue FlValue;

class FlFrame;
// slot with tag which for debug
class FlTagValue{
  FlValue _val;
  uint8_t _tag;
  FlTagValue(uint8_t tag){
    _tag = tag;
  }
  friend FlFrame;

  public:

    FlInt _int()      { return _val._int; }
    FlDouble _double(){ return _val._double; }
    void *objp()      { return _val._obj; }
    FlBool _bool()    { return _val._bool; }
    FlChar _char()    { return _val._char; }

    void set(FlInt v)   { this->_val._int = v; _tag = IntTag; }
    void set(FlBool v)  { this->_val._bool = v; _tag = BoolTag; }
    void set(FlChar v)  { this->_val._char = v; _tag = CharTag; }
    void set(FlDouble v){ this->_val._double = v; _tag = DoubleTag; }
    void set(FlObjp v)   { this->_val._obj = v; _tag = ObjTag; }

    enum TagT {
      IntTag,
      DoubleTag,
      ObjTag,
      BoolTag,
      CharTag,
      UnInit,
    };

    inline std::string toString(){
      switch(_tag){
        case IntTag:    return std::to_string(_int());
        case DoubleTag: return std::to_string(_double());
        case ObjTag:    return "obj";
        case BoolTag:   return std::to_string(_bool());
        case CharTag:   return std::to_string(_char());
        default:        return "nil";
      };
    }
};
class FlString : FlObj{
  const FlChar *chars;
  FlInt len;
  FlInt hash_code;
  public:
    FlString(FlChar *chars,size_t _len){
      this->chars = chars;
      len = _len;
      hash_code = 0;
    }

    FlInt length() {
      return len;
    }

    FlChar charAt(FlInt loc) {
      range_check(loc);
      return chars[loc];
    }

    inline uint64_t hash() override {
      if(hash_code == 0)
        hash_code = hash_cstr(chars, len);
      return hash_code;
    }

    void range_check(FlInt loc){
      if(loc >= len)
        throw "range out of index";
    }
};

// all global
class FlConstPool {
  std::unordered_map<uint64_t, FlTagValue *> _hash_map;
  public:
    FlTagValue * lookup(FlString *key) {
      auto iter = _hash_map.find(key->hash());
      return (iter == _hash_map.end() ? nullptr : iter->second);
    }

    void *put(FlString *key, FlTagValue *value){ _hash_map[key->hash()] = value; }
};

class FlStringConstPool;
class FlVM {
  static FlStringConstPool *const_string_pool;
  static FlConstPool *const_pool;
  public:
  inline static FlString *ofString(const char *chars, size_t len);
  inline static void init();
  static void error(const char *chars){
    COLOR_PRINT(chars, TermColor::Red);
  }
  enum State {
    Uninit,
    Init,
  };
  private:
  static State _state;
  static void state_check(){
    if(_state == Uninit){
      error("FlVM not init yet\n");
      exit(1);
    }
  }
};
// vm static define
FlStringConstPool *FlVM::const_string_pool = nullptr;
FlVM::State FlVM::_state = State::Uninit;

// vm obj

// pre declaration
class FlMethodBuilder;

class FlMethod {
  instr_t *codes;
  size_t _max_stk;
  size_t _max_locals;
  size_t _code_len;
  friend FlFrame;
  friend FlMethodBuilder;
  public:
    size_t max_stk()    { return _max_stk; }
    size_t max_locals() { return _max_locals; }
};

class FlMethodBuilder {
  instr_t *code_cache;
  size_t capability;
  size_t len;
  size_t max_stk;
  size_t max_locals;
  void capability_check(){
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
public:
  FlMethodBuilder(){
    code_cache = nullptr;
    capability = 16;
    len = 0;
    max_stk = -1;
    max_locals = -1;
    code_cache = (instr_t *) malloc(sizeof(instr_t) * capability);
  }

  void clear(){
    len = 0;
    max_stk = -1;
    max_locals = -1;
  }

  FlMethodBuilder *append(instr_t instr){
    capability_check();
    code_cache[len++] = instr;
    return this;
  }

  FlMethodBuilder *set_max_stk(size_t stk_deep){
    max_stk = stk_deep;
    return this;
  }

  FlMethodBuilder *set_max_locals(size_t locals_size){
    max_locals = locals_size;
    return this;
  }

  FlMethod *build(){
    FlMethod *ret = new FlMethod();
    ret->codes = code_cache;
    ret->_max_locals = max_locals;
    ret->_max_stk = max_stk;
    ret->_code_len = this->len;
    clear();
    return ret;
  }

};

class FlStringConstPool {
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

class FlFileLoader {
};

class FlExec;

inline std::string repeat(std::string str, size_t n){
  std::string ret;
  for(int i=0; i<n; i++)
    ret += str;
  return ret;
}

inline std::string append_eq_util(std::string str, size_t n){
  return str + repeat(" ", n - str.size() - 1) + "=";
}

class FlFrame {
  FlFrame *last;
  FlMethod *current_exec;
  instr_t *pc;
  FlTagValue *locals;
  FlTagValue *stk_base;
  FlTagValue *stk_top;
  FlTagValue *stkp;
  friend FlExec;

  void stkp_out_of_index_check(){
    if(stkp < stk_base || stkp >= stk_top){
      FlVM::error("stack out of index>>\n");
      exit(1);
    }
  }

  void init(){
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

  bool end_of_pc(){
    return (current_exec->codes + current_exec->_code_len) <= pc;
  }

  public:
    FlFrame(FlFrame *_last, FlMethod *_method){
      last = _last;
      current_exec = _method;
      init();
    }

    void pushobj(FlObjp v){ stkp->set(v); stkp++; }
    void pushc(FlChar v)  { stkp->set(v); stkp++; }
    void pushd(FlDouble v){ stkp->set(v); stkp++; }
    void pushb(FlBool v)  { stkp->set(v); stkp++; }
    void pushi(FlInt v)   { stkp->set(v); stkp++; }
    void loadi(FlInt v, size_t location) { locals[location].set(v); }
    void loadd(FlDouble v, size_t location) { locals[location].set(v); }

    FlInt popi() {
      stkp--;
      return stkp->_int();
    }

    FlBool popb(){
      stkp--;
      return stkp->_bool();
    }
    FlDouble popd() { return (--stkp)->_double(); }

    void setLast(FlFrame *frame){
      this->last = frame;
    }

    void print_frame(){
      // printf("=====\n");
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
      size_t max_line_len = std::max(std::max(stks.size(), slocals.size()), head.size()) + 1;
      head += repeat("=", max_line_len - head.size());
      stks = append_eq_util(stks, max_line_len);
      slocals = append_eq_util(slocals, max_line_len);
      printf("%s\n",head.c_str());
      printf("%s\n",stks.c_str());
      printf("%s\n",slocals.c_str());
      printf("%s\n",repeat("=",max_line_len).c_str());
    }
};

FlInt sign_extend(uint16_t v){
  if((v >> 15) == 1){
    return  v | 0xFFFFFFFFFFFF0000;
  }
  return v;
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