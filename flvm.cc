#include <iostream>
#include <unordered_map>
#include "MurmurHash2.cc"

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
  };
};

// instruction type : 1 byte
typedef uint8_t instr_t;

// vm types
typedef long long FlInt;
typedef double FlDouble;
typedef bool FlBool;
typedef char FlChar;

// vm slot
union FlValue {
  FlInt _int;
  FlDouble _double;
  void *_obj;
  FlBool _bool;
  FlChar _char;
};

typedef union FlValue FlValue;

// slot with tag which for debug
class FlTagValue{
  FlValue _val;
  uint8_t _tag;
  FlTagValue(uint8_t tag){
    _tag = tag;
  }

  public:

    FlInt _int()      { return _val._int; }
    FlDouble _double(){ return _val._double; }
    void *objp()      { return _val._obj; }
    FlBool _bool()    { return _val._bool; }
    FlChar _char()    { return _val._char; }

    void set(FlInt v)   { this->_val._int = v; }
    void set(FlBool v)  { this->_val._bool = v; }
    void set(FlChar v)  { this->_val._char = v; }
    void set(FlDouble v){ this->_val._double = v; }
    void set(void *v)   { this->_val._obj = v; }

    enum TagT {
      IntTag,
      DoubleTag,
      ObjTag,
      BoolTag,
      CharTag
    };
};

class FlString {
  const FlChar *chars;
  FlInt len;
  public:
    FlString(FlChar *chars,size_t _len){
      this->chars = chars;
      len = _len;
    }

    FlInt length() {
      return len;
    }

    FlChar charAt(FlInt loc) {
      range_check(loc);
      return chars[loc];
    }

    uint64_t hash(){
      return hash_cstr(chars, len);
    }

    void range_check(FlInt loc){
      if(loc >= len)
        throw "range out of index";
    }
};

class FlStringContsPool {
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
    // pool
    pool.emplace(key, val);
  }

  FlString *get(uint64_t key){
    auto iter = pool.find(key);
    if(iter == pool.end())
      return nullptr;
    return iter->second;
  }

};

// vm obj
class FlField {
};

class FlMethod {
  instr_t *codes;
  public:
    size_t max_stk;
    size_t max_locals;
};

class FlKlass {
};

class FlFrame {
  FlFrame *last;
  FlMethod *current_exec;
  instr_t *pc;
  FlTagValue *locals;
  FlTagValue *stk_base;
  FlTagValue *stk_top;
  FlTagValue *stkp;

  void stkp_out_of_index_check(){
    if(stkp < stk_base || stkp >= stk_top){
      printf("stack out of index>>\n");
      exit(1);
    }
  }

  public:
    FlFrame(FlFrame *_last, FlMethod *_method){
      last = _last;
      current_exec = _method;
    }

    void pushobj(void * v){ stkp->set(v); stkp++; }
    void pushc(FlChar v)  { stkp->set(v); stkp++; }
    void pushd(FlDouble v){ stkp->set(v); stkp++; }
    void pushb(FlBool v)  { stkp->set(v); stkp++; }
    void pushi(FlInt v)   { stkp->set(v); stkp++; }

    FlInt popi() {
      stkp--;
      return stkp->_int();
    }

    FlBool popb(){
      stkp--;
      return stkp->_bool();
    }
};

class FlExec {
  FlFrame *base_frame;
  FlFrame *current_frame;

  void _iconst_1(){ current_frame->pushi(1); }
  void _iconst_2(){ current_frame->pushi(2); }
  void _iconst_3(){ current_frame->pushi(3); }
  void _iconst_4(){ current_frame->pushi(4); }
  public:
    void dispatch(instr_t instr){
      switch(instr){
        case Instruction::iconst_1: return _iconst_1();
        case Instruction::iconst_2: return _iconst_2();
        case Instruction::iconst_3: return _iconst_3();
        case Instruction::iconst_4: return _iconst_4();
      };
    }
};
