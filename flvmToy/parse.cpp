#include "parse.h"
#include "lex.h"
#include <functional>

// **************************** token **************************
void Parser::try_fill_cache()
{
  if (_m_tok_cache.empty() && _m_lex.has_token())
  {
    for (int i = 0; i < Parser_Token_Cache_Limit; i++)
    {
      if (!_m_lex.has_token()) break;
      _m_tok_cache.push_back(_m_lex.next_token());
    }
  }
}

token_t Parser::token()
{
  try_fill_cache();
  return _m_tok_cache.empty() ? InvalidTok : _m_tok_cache.front();
}

token_t Parser::next_tok()
{
  try_fill_cache();
  if (_m_tok_cache.empty())
    return InvalidTok; 
  token_t tok = _m_tok_cache.front();
  _m_tok_cache.pop_front();
  return tok;
}

token_t Parser::next_tok_must(TokenKind tk)
{
  token_t t = next_tok();
  //printf("next token is => %s\n", token_to_str(t).c_str());
  if (!token_is_kind(t, tk))
  {
    std::cout << token_to_str(t) << std::endl;
    throw_exception("token is not expected kind");
  }
  return t;
}

token_t Parser::eat(TokenKind tk)
{
  token_t t = next_tok();
  if (!token_is_kind(t, tk))
  {
    std::cout << token_to_str(t) << std::endl;
    throw_exception("token is not expected kind");
  }
  return t;
}

bool Parser::has_tok()
{
  return !_m_tok_cache.empty() || _m_lex.has_token();
}

void Parser::ignore_empty_line()
{
  while(has_tok() && token_is_kind(token(), TokEol))
    next_tok();
}
// ************************* parsing *****************************

IRNode* Parser::parse()
{
  if (has_tok()) return parsing_block();
  printf("does not have token\n");
  return nullptr;
}

IRNode* Parser::parsing_num()
{
  token_t t = next_tok();
  IR_Num* node = new IR_Num(t, tok_foffset(t), tok_end(t));
  unistr_t str = _m_lex.token_string(t);
  std::string u8s;
  str.toUTF8String(u8s);
  switch (token_kind(t)) {
  case TokInt:
    node->_m_num.i_value = std::stoi(u8s);
    break;
  case TokFloat:
    node->_m_num.db_value= std::stod(u8s);
    break;
  }
  return node;
}

IRNode* Parser::parsing_id()
{
  token_t&& t = next_tok();
  return new IR_Id(t, tok_foffset(t), tok_end(t));
}

IRNode* Parser::parsing_str()
{
  token_t t = next_tok();
  unistr_t ustr = _m_lex.token_string(t);
  FlString* str = m_context->string_pool()->of_string(ustr.getBuffer(), ustr.length());
  return new IR_Str(t, tok_end(t), str);
}

IRNode* Parser::parsing_one()
{
  TokenKind tk = token_kind(token());
  switch (tk)
  {
  case TokId:
    return parsing_id();
  case TokInt:
  case TokFloat:
    return parsing_num();
  case TokStr: 
    return parsing_str();
  case TokLParent:
  {
    next_tok();
    IRNode* expression = parsing_comp();
    next_tok_must(TokRParent);
    return expression;
  }
  }
  printf("not support token kind for literal : %s\n", tk_to_str(tk).c_str());
  throw_exception("not support kind literal");
}

IRNode* Parser::binary_parsing_proccess(
  std::function<bool(token_t)> fn_token_is_right_kind,
  std::function<IRNode*()> fn_xhand_side_parsing)
{
  IRNode* lhs = fn_xhand_side_parsing();
  token_t operator_tok;
  while (has_tok() && fn_token_is_right_kind(token()))
  {
    operator_tok = next_tok();
    if (!has_tok())
    {
      printf("doesn't has token when prccess right-hand side");
      throw_exception("parsing binary node exception[ has no right-hand side expression ]");
    }
    IRNode* rhs = fn_xhand_side_parsing();
    lhs = new IR_BinOp(
      operator_tok, 
      tok_end(operator_tok),
      token_kind_to_tag(token_kind(operator_tok)),
      lhs, rhs);
  }
  return lhs;
}

bool token_is_mul(token_t t)
{
  switch (token_kind(t))
  {
  case TokDiv:
  case TokMul:
    return true;
  }
  return false;
}

IRNode* Parser::parsing_mul()
{
  return binary_parsing_proccess(&token_is_mul, [this]() { return parsing_one(); });
}

IRNode* Parser::parsing_comp()
{
  IRNode* lhs = parsing_add();
  switch (token_kind(token()))
  {
  case TokLt:
  case TokLe:
  case TokGt:
  case TokGe:
  case TokEq:
    break;

  default: return lhs;
  }
  token_t token = next_tok();
  IRNode* rhs = parsing_add();
  return new IR_BinOp(token, cur_pos(), token_kind_to_tag(token_kind(token)), lhs, rhs);
}

bool token_is_add(token_t tok)
{
  switch (token_kind(tok))
  {
  case TokAdd:
  case TokSub:
    return true;
  }
  return false;
}

IRNode* Parser::parsing_add()
{
  return binary_parsing_proccess(&token_is_add, [this]() { return parsing_mul(); });
}

IRNode* Parser::parsing_decl()
{
  token_t let = next_tok_must(TokLet);
  token_t id = next_tok_must(TokId);
  next_tok_must(TokAssign);
  IRNode* init = parsing_comp();
  return new IR_Decl(let, tok_end(let), init, _m_lex.token_string(id));
}

IRNode* Parser::parsing_one_line()
{
  switch (token_kind(token()))
  {
    case TokLet:	return parsing_decl();
    default:			return parsing_comp();
  }
}

IRNode* Parser::parsing_block()
{
  IR_Block::state_list_t ls;
  token_t&& start_token = token();
  ignore_empty_line();
  while (has_tok() && !token_is_kind(token(), TokRBrace)) {
    ls.push_back(sptr_t<IRNode>(parsing_stmt()));
    ignore_empty_line();
  }
  return new IR_Block(start_token, _m_lex.cur_pos(), ls);
}

IRNode* Parser::parsing_braced_block()
{
  token_t t = eat(TokLBrace);
  ignore_empty_line();
  if (token_is_kind(token(), TokRBrace)) {
    eat(TokRBrace);
    return new IR_Block(t, _m_lex.cur_pos(), {});
  }
  IRNode* block = parsing_block();
  ignore_empty_line();
  eat(TokRBrace);
  return block;
}

IRNode* Parser::parsing_stmt_if()
{
  token_t&& stmt_if_token = eat(TokIf);
  ignore_empty_line();
  IRNode* test = parsing_comp();
  ignore_empty_line();
  IRNode* success = parsing_braced_block();
  ignore_empty_line();
  if(!token_is_kind(token(), TokElse)) {
    return new IR_If(stmt_if_token, _m_lex.cur_pos(), test, success);
  }
  eat(TokElse);
  ignore_empty_line();
  auto&& parsing_else = [this]() {
    switch(token_kind(token()))
    {
      case TokIf: return parsing_stmt_if();
      default:    return parsing_braced_block();
    }
  };
  IRNode* failed = parsing_else();
  return new IR_If(stmt_if_token, _m_lex.cur_pos(), test, success, failed);
}

IRNode* Parser::parsing_stmt()
{
  TokenKind tk = token_kind(token());
  switch((tk)) {
    case TokLet: return parsing_decl();
    case TokIf:  return parsing_stmt_if();
    case TokFn:  return parsing_fn();
    default: 
      printf("not support stmt that start with %03x %s\n", tk, tk_to_str(tk).c_str());
      throw_exception("");
  }
}

IR_Fn* Parser::parsing_fn()
{
  token_t fn = eat(TokFn);
  bool has_id = token_is_kind(token(), TokId);
  token_t id;
  if(has_id){
    id = next_tok();
  }
  std::vector<sptr_t<IR_FnParam>> params = parsing_fn_params();
  eat(TokColon);
  token_t ret_type = eat(TokId);
  ignore_empty_line();
  IRNode* body = parsing_braced_block();
  unistr_t return_id;
  m_context->find_tok_str(ret_type, return_id);
  if(has_id) {
    unistr_t idStr;
    m_context->find_tok_str(id, idStr);
    return new IR_Fn(fn, _m_lex.cur_pos(), idStr, params, return_id, body);
  } 
  return new IR_Fn(fn, _m_lex.cur_pos(), params, return_id, body);
}

std::vector<sptr_t<IR_FnParam>> Parser::parsing_fn_params()
{
  token_t t = eat(TokLParent);
  std::vector<sptr_t<IR_FnParam>> params;
  if(!token_is_kind(token(), TokRParent)) {
    params.push_back(sptr_t<IR_FnParam>(parsing_param_element()));
  }
  while(has_tok() && !token_is_kind(token(), TokRParent)) {
    eat(TokComma);
    params.push_back(sptr_t<IR_FnParam>(parsing_param_element()));
  }
  if(has_tok()) eat(TokRParent);
  return params;
}

IR_FnParam* Parser::parsing_param_element()
{
  token_t id = eat(TokId);
  eat(TokColon);
  token_t type_id = eat(TokId);
  unistr_t idStr;
  unistr_t typeStr;
  m_context->find_tok_str(id, idStr);
  m_context->find_tok_str(type_id, typeStr);
  return new IR_FnParam(id, _m_lex.cur_pos(), idStr, typeStr);
}

//************************** TypeConvert ***********************
IRNode* TypeConvert::convert(IRNode* ir)
{
  if (nullptr == ir) return ir;
  return ir->accept(*this);
}

IRNode* TypeConvert::visit(IR_Num* num)
{
  if (num->is_float()) {
    push_t(CodeGen_D);
  }
  if (num->is_int()) {
    push_t(CodeGen_I);
  }
  return num;
}

IRNode* TypeConvert::visit(IR_Str* str)
{
  push_t(CodeGen_S);
  return str;
}

IRNode* TypeConvert::visit(IR_BinOp* ir)
{
  CodeGenType rhs_t = pop_t();
  CodeGenType lhs_t = pop_t();
  if (lhs_t != rhs_t) {
    CodeGenType t = better_type(lhs_t, rhs_t);
    push_t(t);
    if (t == lhs_t) {
      IRNode* irRHS = ir->rhs();
      IRNode* rhs = new IR_Cast(irRHS->end_loc(), irRHS, NodeDouble);
      return new IR_BinOp(ir->token(), ir->end_loc(), ir->tag(), ir->lhs(), rhs);
    }
    else {
      IRNode* irLHS = ir->lhs();
      IRNode* lhs = new IR_Cast(irLHS->end_loc(), irLHS, NodeDouble);
      return new IR_BinOp(ir->token(), ir->end_loc(), ir->tag(), lhs, ir->rhs());
    }
  }
  push_t(lhs_t);
  return ir;
}

IRNode* TypeConvert::visit(IR_Cast* cast) 
{ 
  pop_t();
  if (cast->cast_to() == NodeInt) {
    push_t(CodeGen_I);
  }
  if (cast->cast_to() == NodeDouble) {
    push_t(CodeGen_D);
  }
  return cast; 
}

IRNode* TypeConvert::visit(IR_Id* id)
{
  push_t(id_node_type(id));
  return id;
}

IRNode* TypeConvert::visit(IR_Decl* decl)
{
  switch (pop_t()) {
  case CodeGen_D: 
    this->decl(decl->id(), NodeDouble);
    break;
  case CodeGen_I: 
    this->decl(decl->id(), NodeInt);
    break;
  case CodeGen_B:
    this->decl(decl->id(), NodeBool);
    break;
  case CodeGen_S:
    this->decl(decl->id(), NodeString);
    break;
  default:
    std::cout << "decl " << decl->id() << " wrong" << std::endl;
    throw_exception("declare wrong");
  }
  return decl;
}

IRNode* TypeConvert::visit(IR_Block* block)
{
  return block;
}

// TODO:?
IRNode* TypeConvert::visit(IR_If* stmt_if)
{
  stmt_if->test()->accept(*this);
  stmt_if->success()->accept(*this);
  if(stmt_if->has_failed())
    stmt_if->failed()->accept(*this);
  return stmt_if;
}

IRNode* TypeConvert::visit(IR_Fn* stmt_fn)
{
  return stmt_fn;
}

// ******************************* CodeGen ********************************

FnProto* CodeGen::buildProto(IRNode* ir) {
  if(nullptr == ir) {
    printf("empty ir tree\n");
    return nullptr;
  }
  ir->accept(*this);
  return fnBuilder.build();
}

void CodeGen::build(IRNode* ir)
{
  if (ir == nullptr) {
    printf("codegen error, ir is nullptr");
    return;
  }
  ir->accept(*this);
}

IRNode* CodeGen::visit(IR_Block* block)
{
  for(auto it : block->list()) {
    it->accept(*this);
  }
  return block;
}

IRNode* CodeGen::visit(IR_Str* node)
{
  FlString* string = node->str();
  uint8_t loc = store_const(string);
  add_instr(Instruction::ldcs);
  add_instr(loc);
  push_t(CodeGen_S);
  return node;
}


IRNode* CodeGen::visit(IR_Num* num)
{
  CodeGenType t = gen_num(num);
  push_t(t);
  return num;
}

IRNode* CodeGen::visit(IR_Cast* cast)
{
  cast->cast_from()->accept(*this);
  CodeGenType from_t = pop_t();
  switch (from_t) 
  {
  case CodeGen_D:
    if (cast->cast_to() == NodeDouble) {
      push_t(from_t);
    }
    else if (cast->cast_to() == NodeInt) {
      push_t(CodeGen_I);
      add_instr(Instruction::d2i);
    }
    else {
      throw_exception("not support cast");
    }
    break;
  case CodeGen_I:
    if (cast->cast_to() == NodeDouble) {
      push_t(CodeGen_D);
      add_instr(Instruction::i2d);
    }
    else if (cast->cast_to() == NodeInt) {
      push_t(CodeGen_I);
    }
    else {
      throw_exception("not support cast");
    }
    break;
  default:
    throw_exception("gen code error when gen cast node");
  }
  return cast;
}

CodeGenType CodeGen::gen_num(IR_Num * ir) // -> CodeGenType
{
  IR_Num* num = ir;
#define InstrIConst(num) Instruction::iconst_ ## num
#define InstrDConst(num) Instruction::dconst_ ## num
  if (num->is_int())
  {
    FlInt iv = num->get_int();
    switch (iv)
    {
    case 0: add_instr(InstrIConst(0)); break;
    case 1: add_instr(InstrIConst(1)); break;
    case 2: add_instr(InstrIConst(2)); break;
    case 3: add_instr(InstrIConst(3)); break;
    case 4: add_instr(InstrIConst(4)); break;
    default:
      if (-32768 <= iv && iv <= 32767) {
        add_instr(Instruction::ipush);
        add_int16_to_code(iv);
      }
      else {
        uint8_t loc = store_const(iv);
        add_instr(Instruction::ldci);
        add_instr(loc);
      }
    }
    return CodeGen_I;
  }
  else if (num->is_float())
  {
    FlDouble dv = num->get_db();
    if (dv == 1.0 || dv == 2.0) {
      auto instr = dv == 1.0 ? InstrDConst(1) : InstrDConst(2);
      add_instr(instr);
    }
    else {
      uint8_t loc =store_const(dv);
      add_instr(Instruction::ldcd);
      add_instr(loc);
    }
    return CodeGen_D;
  }
  throw std::runtime_error("not support literal when gen code");
};

IRNode* CodeGen::visit(IR_BinOp* ir)
{
  ir->lhs()->accept(*this);
  ir->rhs()->accept(*this);
  push_t(gen_bin(ir));
  return ir;
}

inline Instruction::Code tag_to_double_instr(IRNodeTag tag)
{
  switch (tag)
  {
    case IRTag_Add: return Instruction::dadd;
    case IRTag_Sub: return Instruction::dsub;
    case IRTag_Mul: return Instruction::dmul;
    case IRTag_Div: return Instruction::ddiv;
  }
  throw_exception("not support tag convert to instruction");
}

CodeGenType CodeGen::gen_bin(IR_BinOp* ir)
{
  CodeGenType rhs_t = pop_t();
  CodeGenType lhs_t = pop_t();
  auto&& gen_comp_code = [&](Instruction::Code instr, Instruction::Code s_instr) {
    add_instr(s_instr);
    push_t(rhs_t);
    add_instr(instr);
    pop_t();
    add_instr(fnBuilder.code_len() + 5 - 1);
    add_instr(Instruction::iconst_0);
    push_t(CodeGen_I);
    add_instr(Instruction::go);
    add_instr(fnBuilder.code_len() + 3 - 1);
    add_instr(Instruction::iconst_1);
    add_instr(Instruction::i2b);
    pop_t();
    push_t(CodeGen_B);
    lhs_t = CodeGen_B;
  };

  if (lhs_t != rhs_t)
  {
    // shouldn't reach here 
    printf("binary two side type is not equal\n");
    throw_exception("");
  }

#define BinOp_CodeGen(k, instr) 																				\
          switch (ir->tag()) 																						\
          { 																														\
            case IRTag_Add: add_instr(Instruction:: k ## add); break;         \
            case IRTag_Sub: add_instr(Instruction:: k ## sub); break; 				\
            case IRTag_Mul: add_instr(Instruction:: k ## mul); break; 				\
            case IRTag_Div: add_instr(Instruction:: k ## div); break; 				\
            case IRTag_Eq: gen_comp_code(Instruction::ifeq, Instruction::  instr); break; \
            case IRTag_Lt: gen_comp_code(Instruction::iflt, Instruction::  instr); break; \
            case IRTag_Le: gen_comp_code(Instruction::ifle, Instruction::  instr); break; \
            case IRTag_Ge: gen_comp_code(Instruction::ifge, Instruction::  instr); break; \
            case IRTag_Gt: gen_comp_code(Instruction::ifgt, Instruction::  instr); break; \
            default: 																										\
              printf(">> not support tag type for %x when gen code for binop node", ir->tag()); \
              throw_exception("not support tag convert to instruction"); \
          } 

	switch (lhs_t)
	{
		case CodeGen_I:
			BinOp_CodeGen(i, isub)
			break;
		case CodeGen_D: 
			BinOp_CodeGen(d, dcmp)
			break;
		default:
			throw_exception("not support other type when codegen");
	}
	return lhs_t;
}

IRNode* CodeGen::visit(IR_Id* id)
{
  unistr_t id_str; 
  find_node_str(id, id_str);
  CodeGenType t = variable_t(id_str);
  push_t(t);
  uint8_t local = local_for_name(id_str);
  switch (t)
  {
    case CodeGen_D: add_instr(Instruction::dload); break;
    case CodeGen_I: add_instr(Instruction::iload); break;
    default: throw_exception("not support type when gen code for id node");
  }
  add_instr(local);
  return id;
}

IRNode* CodeGen::visit(IR_Decl* decl)
{
  decl->init()->accept(*this);
  CodeGenType t = pop_t();
  unistr_t name = decl->id();
#define Case_Decl(_case, instr, variable_t) \
  case CodeGen_## _case: \
    add_instr(Instruction:: instr); \
    add_instr(local_for_name(name));  \
    decl_variable(name, variable_t);  \
    break;

  switch (t)
  {
    Case_Decl(I, istore, NodeInt)
    Case_Decl(D, dstore, NodeDouble)
    Case_Decl(B, bstore, NodeBool)
    Case_Decl(S, sstore, NodeString)
    default: throw_exception("encounter unsupported type when gen code for decl");
  }
  return decl;
}

IRNode* CodeGen::visit(IR_If* stmt_if)
{
  stmt_if->test()->accept(*this);
  add_instr(Instruction::ifeq);
  size_t failed_replace_flag = add_instr(0);
  stmt_if->success()->accept(*this);
  if(stmt_if->has_failed()) {
    auto&& failed = stmt_if->failed();
    add_instr(Instruction::go);
    size_t jump_failed_flag = add_instr(0);
    replace_instr(failed_replace_flag, fnBuilder.code_len());
    failed->accept(*this);
    replace_instr(jump_failed_flag, fnBuilder.code_len());
  }else {
    replace_instr(failed_replace_flag, fnBuilder.code_len());
  }
  return stmt_if;
}

IRNode* CodeGen::visit(IR_Fn* stmt_fn)
{
  return stmt_fn;
}
