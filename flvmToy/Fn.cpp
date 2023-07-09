#include "Fn.h"

#define PutInConst(v) \
  FlTagValue value; \
  value.set(v);     \
  k_cache.push_back(value) 

uint8_t FnProtoBuilder::store_int_const(FlInt intValue) {
  PutInConst(intValue);
  return k_cache.size();
}

uint8_t FnProtoBuilder::store_double_const(FlDouble doubleValue) {
  PutInConst(doubleValue);
  return k_cache.size();
}

uint8_t FnProtoBuilder::store_str_const(FlString* str) {
  PutInConst(str);
  return k_cache.size();
}

FnProtoBuilder* FnProtoBuilder::append(instr_t instr) {
  code_cache.push_back(instr);
  return this;
}

FnProtoBuilder* FnProtoBuilder::replace(size_t loc, instr_t instr) {
  if(loc >= code_cache.size()) {
    throw std::exception("");
  }
  code_cache[loc] = instr;
  return this;
}

size_t FnProtoBuilder::code_len() {
  return code_cache.size();
}

FnProto* FnProtoBuilder::build() {
  FnProto* proto = new FnProto();
  size_t clen = code_len();
  proto->code = new instr_t[clen];
  for(int i=0; i < clen ; i++) {
    proto->code[i] = code_cache[i];
  }
  size_t klen = k_cache.size();
  proto->k = new FlTagValue[klen];
  for(int i=0; i < klen; i++) {
    proto->k[i] = k_cache[i];
  }
  return proto;
}
