#pragma once
#include <stdint.h>
#include <memory>
#include <string>

#include "unicode/uchar.h"
#include "unicode/ustring.h"

class FlObj {
  virtual uint64_t hash() = 0;
};


typedef long long FlInt;
typedef double    FlDouble;
typedef bool      FlBool;
typedef UChar      FlChar;
typedef uint8_t   FlByte;
typedef FlObj*    FlObjp;
// vm slot
typedef union FlValue {
  FlInt _int;
  FlDouble _double;
  FlObjp _obj;
  FlBool _bool;
  FlChar _char;
  FlByte _byte;
} FlValue;

class FlTagValue{
  FlValue _val;
  uint8_t _tag;
  FlTagValue(uint8_t tag){
    _tag = tag;
    _val._int = 0;
  }
  FlTagValue() : _tag(0) {}
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

    enum TagT : uint8_t{
      IntTag,
      DoubleTag,
      ObjTag,
      BoolTag,
      CharTag,
      UnInit,
    };
    inline const FlValue& union_v() const { return _val; }
    inline TagT tag()  const { return (TagT)_tag; }

    inline void toString(std::string& out){
      switch(_tag){
      case IntTag:    out = std::move(std::to_string(_int()));
      case DoubleTag: out = std::move(std::to_string(_double()));
      case ObjTag:    out = ("obj");
      case BoolTag:   out = std::move(std::to_string(_bool()));
      case CharTag:   out = std::move(std::to_string(_char()));
        default:      out = "nil";
      };
    }
};

std::ostream & operator<<(std::ostream& stream, const FlTagValue& value);
uint64_t hash_cstr(const char *p, size_t len);

class FlString : FlObj {
  const FlChar *chars;
  FlInt len;
  FlInt hash_code;
  public:
    FlString(FlChar *chars,size_t _len){
      // this->chars = chars;
      this->chars = new UChar[_len];
      // U_ICU_NAMESPACE::
      u_strncpy(const_cast<UChar*>(this->chars), chars, _len);
      len = _len;
      hash_code = 0;
    }

    ~FlString(){ if(chars != nullptr) delete[] chars; }

    FlInt length() { return len; }

    FlChar charAt(FlInt loc) {
      range_check(loc);
      return chars[loc];
    }

    inline uint64_t hash() override {
      if(hash_code == 0) {
        const char* cs = (const char*) chars;
        hash_code = hash_cstr(cs, len * 2);
      }
      return hash_code;
    }

    void range_check(FlInt loc){
      if(loc >= len)
        throw "range out of index";
    }

};

class StringPool 
{
public:
  FlString of_string(const UChar* cs);
  FlString of_string(FlString* str);
private:
  std::unordered_map<uint64_t, FlString*> m_string_map;
};