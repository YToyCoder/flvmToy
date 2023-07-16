#include "Fn.h"
#include <iostream>
#include <stdio.h>
#define STR(c) #c
#define CasePrint(instruction) \
  case Instruction::## instruction ##: \
    printf(STR(%03d %04x %10s\n), ic_record, instr, #instruction); \
    break

#define CasePrintOneParam(instruction) \
  case Instruction::## instruction ##: \
  printf(STR(%03d %04x %10s %04d \n), ic_record, instr, #instruction, code[instr_cursor++]); \
  break

void FnProto::to_string() const {
  size_t instr_cursor = 0;
  instr_t instr;
  int ic_record;
  for(int i = 0; i < max_k_size; i++) {
    printf("%02d ", i);
    std::cout << k[i] << std::endl;
  }
  while(instr_cursor < code_len) {
    ic_record = instr_cursor;
    instr = code[instr_cursor++];
    switch(instr) {
      CasePrint(iconst_0);
      CasePrint(iconst_1);
      CasePrint(iconst_2);
      CasePrint(iconst_3);
      CasePrint(iconst_4);
      CasePrint(dconst_1);
      CasePrint(dconst_2);
      case Instruction::ipush: {
        uint8_t i1 = code[instr_cursor++];
        uint8_t i2 = code[instr_cursor++];
        printf("%03d %04x %10s %03ll\n", ic_record, instr, "ipush", sign_extend(i1 | ( i2 << 8) ));
      }
      break;
      CasePrintOneParam(dpush);
      CasePrintOneParam(iload);
      CasePrintOneParam(dload);
      CasePrint(iadd);
      CasePrint(dadd);
      CasePrint(isub);
      CasePrint(dsub);
      CasePrint(imul);
      CasePrint(dmul);
      CasePrint(idiv);
      CasePrint(ddiv);
      case Instruction::ldci: 
      {
        uint8_t const_loc = code[instr_cursor++];
        printf("%03d %04x %10s #%03d ", ic_record, instr, "ldci", const_loc);
        std::cout << k[const_loc] << std::endl;
      }
      break;
      case Instruction::ldcd: 
      {
        uint8_t const_loc = code[instr_cursor++];
        printf("%03d %04x %10s #%03d ", ic_record, instr, "ldcd", const_loc);
        std::cout << k[const_loc] << std::endl;
      }
      break;
      case Instruction::ldcs: 
      {
        uint8_t const_loc = code[instr_cursor++];
        printf("%03d %04x %10s #%03d ", ic_record, instr, "ldcd", const_loc);
        std::cout << k[const_loc] << std::endl;
      }
      break;
      CasePrint(i2d);
      CasePrint(i2b);
      CasePrint(d2i);
      CasePrint(dcmp);
      CasePrintOneParam(istore);
      CasePrintOneParam(dstore);
      CasePrintOneParam(bstore);
      CasePrintOneParam(ostore);
      CasePrintOneParam(ifeq);
      CasePrintOneParam(iflt);
      CasePrintOneParam(ifle);
      CasePrintOneParam(ifgt);
      CasePrintOneParam(ifge);
      CasePrintOneParam(go);
      default:
        printf("not support instruction (%04x:%s) to string\n", instr, instr_name((Instruction::Code)instr));
        throw std::exception("not support instruction when code to string");
    }
  }
}

#define PutInConst(v) \
  FlTagValue value; \
  value.set(v);     \
  k_cache.push_back(value) 

uint8_t FnProtoBuilder::store_int_const(FlInt intValue) {
  PutInConst(intValue);
  return k_cache.size() - 1;
}

uint8_t FnProtoBuilder::store_double_const(FlDouble doubleValue) {
  PutInConst(doubleValue);
  return k_cache.size() - 1;
}

uint8_t FnProtoBuilder::store_str_const(FlString* str) {
  PutInConst(str);
  return k_cache.size() - 1;
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
  size_t clen = proto->code_len = code_len();
  proto->code = new instr_t[clen];
  for(int i=0; i < clen ; i++) {
    proto->code[i] = code_cache[i];
  }
  size_t klen = proto->max_k_size = k_cache.size();
  proto->k = new FlTagValue[klen];
  for(int i=0; i < klen; i++) {
    proto->k[i] = k_cache[i];
  }
  return proto;
}
