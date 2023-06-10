#pragma once
#include "stdint.h"
#include <string>
#include <unordered_map>

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

void COLOR_PRINT(const char* s, int color);
uint64_t hash_cstr(const char *p, size_t len);

struct Instruction {
  enum Code {
    iconst_0 = 0x00,
    iconst_1 = 0x01,
    iconst_2 = 0x02,
    iconst_3 = 0x03,
    iconst_4 = 0x04,
    dconst_1 = 0x05,
    dconst_2 = 0x06,
    ipush    = 0x07,
    dpush    = 0x08,
    iload    = 0x09,
    dload    = 0x0A,
    aload    = 0x0B,
    ldci     = 0x11, // load int from consts
    ldcd     = 0x12, // load double from consts

    // operator
    iadd     = 0x21,
    dadd     = 0x22,
    isub     = 0x23,
    dsub     = 0x24,
    imul     = 0x25,
    dmul     = 0x26,
    idiv     = 0x27,
    ddiv     = 0x28,
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

FlInt sign_extend(uint16_t v);
// slot with tag which for debug
class FlTagValue{
  FlValue _val;
  uint8_t _tag;
  FlTagValue(uint8_t tag){
    _tag = tag;
    _val._int = 0;
  }
  friend class FlFrame;

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

    const char* c_char(){
      return chars;
    }
};

class FlMethod {
  instr_t* codes;
  FlValue* k;   // const value in method
  size_t _max_stk;
  size_t _max_locals;
  size_t _code_len;
  size_t _k_len; // debug
  friend class FlFrame;
  friend class FlMethodBuilder;
  public:
    size_t max_stk()    { return _max_stk; }
    size_t max_locals() { return _max_locals; }
};

class FlFrame
{
public:

    void pushobj(FlObjp v){ stkp->set(v); stkp++; }
    void pushc(FlChar v)  { stkp->set(v); stkp++; }
    void pushd(FlDouble v){ stkp->set(v); stkp++; }
    void pushb(FlBool v)  { stkp->set(v); stkp++; }
    void pushi(FlInt v)   { stkp->set(v); stkp++; }
    void loadi(FlInt v, size_t loc) { locals[loc].set(v); }
    void loadd(FlDouble v, size_t loc) { locals[loc].set(v); }

    void iadd() { pushi(popi() + popi()); }
    void dadd() { pushd(popd() + popd()); }
    void isub() { pushi(popi() - popi()); }
    void dsub() { pushd(popd() - popd()); }
    void imul() { pushi(popi() * popi()); }
    void dmul() { pushd(popd() * popd()); }
    void idiv() { pushi(popi() / popi()); }
    void ddiv() { pushd(popd() / popd()); }

    FlInt popi() { stkp--; return stkp->_int(); }
    FlBool popb(){ stkp--; return stkp->_bool();}
    FlDouble popd() { return (--stkp)->_double(); }

    void ldci(uint8_t k_loc) { pushi(current_exec->k[k_loc]._int);    }
    void ldcd(uint8_t k_loc) { pushd(current_exec->k[k_loc]._double); }

public:
    void print_frame();
    void init();
    void setLast(FlFrame *frame){ this->last = frame; }
    bool end_of_pc() { (current_exec->codes + current_exec->_code_len) <= pc; }

private:  
  inline void stkp_out_of_index_check();

  FlFrame *last;
  FlMethod *current_exec;
  instr_t *pc;
  FlTagValue *locals;
  FlTagValue *stk_base;
  FlTagValue *stk_top;
  FlTagValue *stkp;
  friend class FlExec;
  friend class FlSExec;
};

// all global
class FlConstPool {
  std::unordered_map<uint64_t, FlTagValue *> _hash_map;
  public:
    FlTagValue * lookup(FlString *key) {
      auto iter = _hash_map.find(key->hash());
      return (iter == _hash_map.end() ? nullptr : iter->second);
    }

    void put(FlString *key, FlTagValue *value){ _hash_map[key->hash()] = value; }
};

// a simple executor
class FlSExec
{

protected:
  inline instr_t read_instr(){ return *(_m_frame->pc++);}
  inline void _iconst_0(){ _m_frame->pushi(1); }
  inline void _iconst_1(){ _m_frame->pushi(1); }
  inline void _iconst_2(){ _m_frame->pushi(2); }
  inline void _iconst_3(){ _m_frame->pushi(3); }
  inline void _iconst_4(){ _m_frame->pushi(4); }
  inline void _iload() { _m_frame->loadi(_m_frame->popi(), read_instr());}
  inline void _dpush() { _m_frame->pushd(read_instr()); }
  inline void _ipush() { 
    FlInt v = sign_extend(read_instr() << 8 | read_instr());
    _m_frame->pushi(v);
  }

  void dispatch(instr_t instr);

private:
  FlFrame* _m_frame;
};

class FlMethodBuilder {
public:
  FlMethodBuilder();
  ~FlMethodBuilder();

  FlMethodBuilder* append(instr_t instr);
  FlMethodBuilder* store_const_int(FlInt _i);
  FlMethodBuilder* set_max_stk(size_t stk_deep);
  FlMethodBuilder* set_max_locals(size_t locals_size);
  FlMethod* build();

  void clear()
  {
    len = 0;
    max_stk = -1;
    max_locals = -1;
  }
private:
	void capability_check();
  inline void const_pool_size_check()
  {
    if (k_len == k_cap)
    {
      size_t n_len = 1.5 * k_cap;
      FlValue* n_area = new FlValue[n_len];
      for (int i = 0; i < k_len && i < n_len; i++)
      {
        n_area[i] = k_cache[i];
      }
      delete[] k_cache;
    }
  }

  instr_t *code_cache;
  size_t capability;
  size_t len;
  size_t max_stk;
  size_t max_locals;

  FlValue* k_cache;
  size_t    k_cap;
  size_t    k_len;
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
