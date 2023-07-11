#pragma once
#include "flvm.hpp"

class FnProto
{
public:
  void to_string() const;
  instr_t*  code;            // code
  int32_t   code_len;       // debug

  FlTagValue* k;            // const value
  int32_t   max_k_size;     // debug

  FnProto*  fns;            // functions
  int32_t   fn_size;

  int32_t   max_stk_size;   // stack
  int32_t   max_local_size; // local
};

class FnProtoBuilder 
{
public:
  uint8_t store_int_const(FlInt intValue);
  uint8_t store_double_const(FlDouble doubleValue);
  uint8_t store_str_const(FlString* str);
  FnProtoBuilder* append(instr_t instr);
  FnProtoBuilder* replace(size_t loc, instr_t instr);
  size_t code_len();
  FnProto* build();
private:
  std::vector<instr_t>  code_cache;
  int32_t               max_stk;
  int32_t               max_locals;

  std::vector<FlTagValue> k_cache;
};