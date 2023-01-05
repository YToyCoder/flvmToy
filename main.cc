#include <iostream>
#include <unordered_map>
#include "MurmurHash2.cc"

const time_t hash_seed = time(nullptr);

uint64_t hash_cstr(const char *p, size_t len){
  return MurmurHash64A(p, len, hash_seed);
}

class Instruction {
  enum Code {
    iconst_1 = 0x01,
    iconst_2 = 0x02,
    iconst_3 = 0x03,
    iconst_4 = 0x04,
  };
};

typedef long long FlInt;
typedef double FlDouble;
typedef bool FlBool;
typedef char FlChar;

union FlValue {
  FlInt _int;
  FlDouble _double;
  void *_obj;
  FlBool _bool;
  FlChar _char;
};

typedef union FlValue FlValue;

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

class FlField {
};

class FlMethod {
};

class FlKlass {
};

int main(int argc, char const *argv[])
{
  std::cout << "hello, this is flvm" << std::endl;
  return 0;
}
