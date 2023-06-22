#pragma once
#include <string>
#include <unordered_map>
#include <stdint.h>

#include "FlValue.h"

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

struct Instruction {
  enum Code: uint8_t {
    nop      = 0x00, // do nothing
    iconst_0 = 0x01,
    iconst_1 = 0x02,
    iconst_2 = 0x03,
    iconst_3 = 0x04,
    iconst_4 = 0x05,
    dconst_1 = 0x06,
    dconst_2 = 0x07,
    ipush    = 0x08,
    dpush    = 0x09,
    iload    = 0x0A,
    dload    = 0x0B,
    aload    = 0x0C,
    istore   = 0x0D,
    dstore   = 0x0E,
    bstore   = 0x0F,
    ostore   = 0x11, // store string in local value
    ldci     = 0x15, // load int from consts
    ldcd     = 0x16, // load double from consts
    ldcs     = 0x17, // load string from consts

    // operator
    iadd     = 0x21,
    dadd     = 0x22,
    isub     = 0x23,
    dsub     = 0x24,
    imul     = 0x25,
    dmul     = 0x26,
    idiv     = 0x27,
    ddiv     = 0x28,

    // int comp
    ifeq     = 0x29,
    iflt     = 0x2A,
    ifle     = 0x2B,
    ifgt     = 0x2C,
    ifge     = 0x2D,
    go       = 0x2E,
    dcmp     = 0x2F,

    // cast
    i2d      = 0x30, // int to double
    i2b      = 0x31, // int to boolean
    d2i      = 0x32, // double to int
  };
};

extern const char* instr_map[256];

#define InstrNameDefine(instr) \
  instr_map[(uint8_t)Instruction:: ## instr] = #instr

void set_instr_map();
const char* instr_name(Instruction::Code instr);
// instruction type : 1 byte
typedef uint8_t instr_t;

FlInt sign_extend(uint16_t v);
// slot with tag which for debug

class FlMethod {
  instr_t* codes;
  FlTagValue* k;   // const value in method
  size_t _max_stk;
  size_t _max_locals;
  size_t _code_len;
  size_t _k_len; // debug
  friend class FlFrame;
  friend class FlMethodBuilder;
  public:
    size_t max_stk()    { return _max_stk; }
    size_t max_locals() { return _max_locals; }
    void to_string() const;
};

class FlFrame
{
public:

    void pushobj(FlObjp v){ stkp->set(v); stkp++; }
    void pushc(FlChar v)  { stkp->set(v); stkp++; }
    void pushd(FlDouble v){ stkp->set(v); stkp++; }
    void pushb(FlBool v)  { stkp->set(v); stkp++; }
    void pushi(FlInt v)   { stkp->set(v); stkp++; }
    void pusho(FlObjp v)   { stkp->set(v); stkp++; }
    void loadi(size_t loc) { pushi(locals[loc]._int()); }
    void loadd( size_t loc) { pushd(locals[loc]._double()); }
    void storei(FlInt i, size_t loc) { locals[loc].set(i); }
    void stored(FlDouble i, size_t loc) { locals[loc].set(i); }
    void storeb(FlBool i, size_t loc) { locals[loc].set(i); }
    void storeo(FlObj* obj, size_t loc) { locals[loc].set(obj); }

    void iadd() { pushi(popi() + popi()); }
    void dadd() { pushd(popd() + popd()); }
    void imul() { pushi(popi() * popi()); }
    void dmul() { pushd(popd() * popd()); }
    void idiv() { pushi(popi() / popi()); }
    void ddiv() { pushd(popd() / popd()); }
    void isub() { FlInt i = popi(); pushi(popi() - i); }
    void dsub() { FlDouble d = popd(); pushd(popd() - d); }

    FlInt popi() { pop_check(); stkp--; return stkp->_int(); }
    FlBool popb(){ pop_check();stkp--; return stkp->_bool();}
    FlDouble popd() {pop_check(); return (--stkp)->_double(); }
    FlObj* popo() {pop_check(); return (--stkp)->_objp(); }

    void ldci(uint8_t k_loc) { pushi(current_exec->k[k_loc]._int());    }
    void ldcd(uint8_t k_loc) { pushd(current_exec->k[k_loc]._double()); }
    void ldcs(uint8_t k_loc) { pusho(current_exec->k[k_loc]._objp()); }
    void set_pc(uint8_t offset) { pc = current_exec->codes + offset; }

public:
    void print_frame(int32_t instr);
    void init();
    void setLast(FlFrame *frame){ this->last = frame; }
    bool end_of_pc() { return (current_exec->codes + current_exec->_code_len) <= pc; }

private:  
  inline void stkp_out_of_index_check();
  inline void pop_check()
  {
    if(stkp == stk_base)
    {
      printf("stk break\n");
    }
  }

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

// a simple executor: exec one method
class FlSExec
{
public:
  FlSExec(): _m_frame(nullptr) { }
  void exec(FlMethod* method)
  {
    if (nullptr == method)
    {
      throw std::exception("method is nullptr when exec method");
    }
    FlFrame frame;
    _m_frame = &frame;
    _m_frame->current_exec = method;
    _m_frame->init();
		while(true){
			if(_m_frame->end_of_pc()){
				printf("end of pc\n");
				break;
			}
			dispatch(*(_m_frame->pc++));
		}
  }
protected:
  inline instr_t read_instr(){ return *(_m_frame->pc++);}
  inline void _iconst_0(){ _m_frame->pushi(0); }
  inline void _iconst_1(){ _m_frame->pushi(1); }
  inline void _iconst_2(){ _m_frame->pushi(2); }
  inline void _iconst_3(){ _m_frame->pushi(3); }
  inline void _iconst_4(){ _m_frame->pushi(4); }
  inline void _iload() { _m_frame->loadi(read_instr());}
  inline void _dpush() { _m_frame->pushd(read_instr()); }
  inline void _ipush() { 
    FlInt v = sign_extend(read_instr() | read_instr() << 8 );
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

  uint8_t store_const_int(FlInt _i);
  uint8_t store_const_double(FlDouble _d);
  uint8_t store_const_str(FlString* obj);
  FlMethodBuilder* append(instr_t instr);
  FlMethodBuilder* set_max_stk(size_t stk_deep);
  FlMethodBuilder* set_max_locals(size_t locals_size);
  FlMethod* build();
  inline size_t code_len() const { return len; }
  inline void replace(size_t loc, uint8_t instr) {
    if(loc >= len) throw std::exception();
    code_cache[loc] = instr;
  }

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
      FlTagValue* n_area =(FlTagValue *) malloc(sizeof( FlTagValue) * n_len);
      for (int i = 0; i < k_len && i < n_len; i++)
      {
        n_area[i] = k_cache[i];
      }
      free(k_cache);
      k_cache = n_area;
    }
  }

  instr_t *code_cache;
  size_t capability;
  size_t len;
  size_t max_stk;
  size_t max_locals;

  FlTagValue* k_cache;
  size_t    k_cap;
  size_t    k_len;
};
