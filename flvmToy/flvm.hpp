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

    const char* c_char(){
      return chars;
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

    void put(FlString *key, FlTagValue *value){ _hash_map[key->hash()] = value; }
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