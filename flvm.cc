#include <iostream>
#include <unordered_map>
#include <string>
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
    dconst_1 = 0x05,
    dconst_2 = 0x06,
    ipush    = 0x07,
    iload    = 0x08,
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
    void set(void *v)   { this->_val._obj = v; _tag = ObjTag; }

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

class FlKlass {
};

class FlExec;

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
      printf("stack out of index>>\n");
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

    void pushobj(void * v){ stkp->set(v); stkp++; }
    void pushc(FlChar v)  { stkp->set(v); stkp++; }
    void pushd(FlDouble v){ stkp->set(v); stkp++; }
    void pushb(FlBool v)  { stkp->set(v); stkp++; }
    void pushi(FlInt v)   { stkp->set(v); stkp++; }
    void loadi(FlInt v, size_t location) { locals[location].set(v); }

    FlInt popi() {
      stkp--;
      return stkp->_int();
    }

    FlBool popb(){
      stkp--;
      return stkp->_bool();
    }

    void setLast(FlFrame *frame){
      this->last = frame;
    }

    void print_frame(){
      std::string ret("frame data:\n stk: ");
      for(FlTagValue *i=stk_base; i<stk_top; i++){
        std::string el = i < stkp ? "[" + i->toString() + "]" : i->toString();
        ret += el + " ";
      }
      ret += "\n locals: ";
      for(int i=0; i<current_exec->max_locals(); i++){
        ret += locals[i].toString() + " ";
      }
      ret += "\n";
      printf(ret.c_str());
      printf("print frame\n");
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
