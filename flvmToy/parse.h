#pragma once
#include "lex.h"
#include "ir.h"
#include "Fn.h"
#include "unicode/ustream.h"
#include "flvm.hpp"
#include <list>
#include <set>
#include <map>
#include <iostream>
#include <functional>

#define Parser_Token_Cache_Limit 2

class Parser
{
public:
  Parser(const std::string& filename) 
    :_m_lex(filename), m_context(nullptr) { }
  Parser(): Parser("") {}
  IRNode* parse();
  bool init(Context* c) { 
    set_context(c);
    return _m_lex.init(c); 
  }
protected:
  // ***************************** parsing ********************************
  // num 
  IRNode* parsing_num();
  // id
  IRNode* parsing_id();
  // string
  IRNode* parsing_str();
  // one = 	num
  //			| id 
  //			| string
  //			| "(" cmp ")"
  IRNode* parsing_one(); // the expression that can treat as one expression
  // mul = 	one "*" one
  //			| one "/" one
  IRNode* parsing_mul();
  // add = 	one "+" one
  //			| one "-" one
  IRNode* parsing_add();
  // cmp = add "==" add
  // 			| add "<" add
  // 			| add "<=" add
  // 			| add ">" add
  // 			| add ">=" add
  IRNode* parsing_comp();
  IRNode* binary_parsing_proccess(
    std::function<bool(token_t)> fn_token_is_right_kind,
    std::function<IRNode*()> fn_xhand_side_parsing);

  // "let" id = cmp
  IRNode* parsing_decl();
  IRNode* parsing_block();
  IRNode* parsing_braced_block();
  // one_line = decl "\n" | cmp "\n"
  IRNode* parsing_one_line();
  // stmt = decl | stmt_if
  IRNode* parsing_stmt();
  // stmt_if = "if" cmp braced_block if_rest
  // if_rest =  "" 
  //          | "else" bracked_block 
  //          | "else" stmt_if bracked_block
  IRNode* parsing_stmt_if();
  // stmt_fn = "fn" [ id ] fn_parameters return_t braced_block
  IR_Fn* parsing_fn();
  // fn_parameters = "(" parameter parameter_rest ")"
  std::vector<sptr_t<IR_FnParam>> parsing_fn_params();
  // id : type_id
  IR_FnParam* parsing_param_element();

  // ******************************** token ******************************
  inline token_t 	token(); 
  inline token_t 	next_tok();
  inline token_t 	next_tok_must(TokenKind tk);
  inline token_t 	eat(TokenKind tk);
  inline bool			has_tok(); 
  inline void			try_fill_cache();
  inline uint32_t cur_pos() const { return _m_lex.cur_pos(); }
  inline void 		ignore_empty_line();

  // ********************** context ********************************
  void set_context(Context* context) { m_context = context; }
private:
  std::list<token_t> _m_tok_cache;
  Lex _m_lex;
  Context* m_context;
};

enum CodeGenType
{
  CodeGen_B, // bool ? byte
  CodeGen_I, // int
  CodeGen_D, // double
  CodeGen_S, // string
};

inline CodeGenType better_type(CodeGenType ta, CodeGenType tb)
{
  return ta > tb ? ta : tb;
}
class NameTypeMap
{
public:
  NameTypeMap(Context* c): m_context(c) {}
protected:
  void decl(unistr_t n, unistr_t t)
  {
    auto it = m_name_type_map.find(n);
    if (it != m_name_type_map.end())
    {
      std::cout << "variable " << n << " have been declared " << std::endl;
      #ifdef __GNUC__
        throw std::exception();
      #else 
        throw std::exception("redeclare for variable name");
      #endif
    }
    m_name_type_map.insert(std::make_pair(n, t));
  }

  CodeGenType variable_t(unistr_t n)
  {
    auto it = m_name_type_map.find(n);
    if (it == m_name_type_map.end())
    {
      std::cout << "variable " << n << " not declared" << std::endl;
      #ifdef __GNUC__
        throw std::exception();
      #else 
        throw std::exception("variable not declared");
      #endif
    }
    unistr_t type_name = it->second;
    if (type_name == NodeInt) {
      return CodeGen_I;
    }else if (type_name == NodeDouble) {
      return CodeGen_D;
    }
    else if(type_name == NodeString) {
      return CodeGen_S;
    }
    else {
      std::cout << "not support type for " << type_name << std::endl;
      #ifdef __GNUC__
        throw std::exception();
      #else 
        throw std::exception("not support type");
      #endif
    }
  }

  CodeGenType id_node_type(const IR_Id* id)
  {
    unistr_t str;
    if (!m_context->find_tok_str(id->token(), str))
    {
      #ifdef __GNUC__
        throw std::exception();
      #else 
        throw std::exception("could not find id node token str");
      #endif
    }
    return variable_t(str);
  }

  void find_node_str(const IR_Id* id, unistr_t& str)
  {
    if (!m_context->find_tok_str(id->token(), str))
    {
      #if defined(__GNUC__)
        throw std::exception();
      #else 
        throw std::exception("could not find id node token str");
      #endif
    }
  }

  void find_token_str(token_t tok, unistr_t& str)
  {
    if (!m_context->find_tok_str(tok, str))
    {
      #if defined(__GNUC__)
        throw std::exception();
      #else 
        throw std::exception("could not find id node token str");
      #endif
    }
  }

  std::map<unistr_t, unistr_t> m_name_type_map;
  Context* m_context;
};
class TypeConvert : public IRVisitor, public NameTypeMap
{
public:
  TypeConvert(Context* c) : NameTypeMap(c) { }
  IRNode* convert(IRNode* ir);
private:
  IR_Visitor_Impl_Decl()
  inline void push_t(CodeGenType t) { t_stk.push_back(t); }
  inline CodeGenType pop_t() {
    auto t = t_stk.back();
    t_stk.pop_back();
    return t;
  }
  std::list<CodeGenType> t_stk;
};

class CodeGen: protected Visitor, protected NameTypeMap
{
public:
  CodeGen(Context* c): NameTypeMap(c), m_max_local(0), max_stk_size(0) { }
  void build(IRNode* ir);
  FnProto* buildProto(IRNode* ir);
private:
  IR_Visitor_Impl_Decl()
  
  CodeGenType gen_num(IR_Num* ir);
  CodeGenType gen_bin(IR_BinOp* ir);

// code append operation
  inline void replace_instr(size_t loc, uint8_t instr) { fnBuilder.replace(loc, instr); }
// position
  inline size_t add_instr(uint8_t instr) { fnBuilder.append(instr); return fnBuilder.code_len() - 1; }
  inline void add_int16_to_code(int16_t _i)
  {
    uint8_t* byte = (uint8_t*)(&_i);
    fnBuilder.append(*byte);
    fnBuilder.append(*(byte + 1));
  }
// type operation
  inline CodeGenType pop_t()
  {
    CodeGenType t = t_stk.back();
    t_stk.pop_back();
    return t;
  }
  inline CodeGenType top_t() { return t_stk.back(); }
  inline void push_t(CodeGenType t) 
  { 
    t_stk.push_back(t); 
    set_max_stk_size();
  }

  inline void set_max_stk_size()
  {
    if (t_stk.size() > max_stk_size)
    {
      max_stk_size = t_stk.size();
    }
  }

  // 从本地变量表中申请一个新的本地变量表
  inline uint8_t local_for_name(unistr_t name)
  {
    auto it = m_local_name_map.find(name);
    if (it != m_local_name_map.end())
      return it->second;
    uint8_t local = m_max_local++;
    m_local_name_map.insert(std::make_pair(name, local));
    return local;
  }

  inline uint8_t store_const(FlInt i)
  {
    uint8_t loc;
    if (!lookup_in_map(m_int_const_map, i, loc))
    {
      loc = fnBuilder.store_int_const(i);
      m_int_const_map.insert(std::make_pair(i, loc));
    }
    return loc;
  }

  inline uint8_t store_const(FlDouble d)
  {
    uint8_t loc;
    if (!lookup_in_map(m_double_const_map, d, loc))
    {
      // loc = m_builder.store_const_double(d);
      loc = fnBuilder.store_double_const(d);
      m_double_const_map.insert(std::make_pair(d, loc));
    }
    return loc;
  }

  inline uint8_t store_const(FlString* str)
  {
    uint8_t loc;
    if (!lookup_in_map(m_str_map, str, loc))
    {
      // loc = m_builder.store_const_str(str);
      loc = fnBuilder.store_str_const(str);
      m_str_map.insert(std::make_pair(str, loc));
    }
    return loc;
  }

  template<class _Ty>
  bool lookup_in_map(const std::map<_Ty,uint8_t>& map,_Ty& key, uint8_t& out)
  {
    auto it = map.find(key);
    if (it == map.end())
    {
      return false;
    }
    out = it->second;
    return true;
  }

  inline void decl_variable(unistr_t name, unistr_t t)
  {
    decl(name, t);
    local_for_name(name);
  }

  // type stack
  std::list<CodeGenType> 				t_stk; // type stack
  size_t 												max_stk_size;
  // local info
  uint8_t 											m_max_local; // init 0
  std::set<uint8_t> 						m_unused_local;
  std::map<unistr_t, uint8_t> 	m_local_name_map;   // string map to local index
  std::map<FlInt, uint8_t> 			m_int_const_map;			
  std::map<FlDouble, uint8_t> 	m_double_const_map; //
  std::map<FlString*, uint8_t> 	m_str_map; 	
  FnProtoBuilder                fnBuilder; // function builder
};
