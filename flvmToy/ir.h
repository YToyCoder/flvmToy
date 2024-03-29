#pragma once
#include "common.h"
#include "token.h"
#include "FlValue.h"
#include "unicode/ustream.h"

#include <memory>
#include <vector>
#include <sstream>

enum IRNodeTag {
  IRTag_Id = 1,
  IRTag_Num,
  IRTag_Str,
  IRTag_Add,
  IRTag_Sub,
  IRTag_Mul,
  IRTag_Div,
  IRTag_Eq, // ==
  IRTag_Lt, // <
  IRTag_Le, // <=
  IRTag_Gt, // >
  IRTag_Ge, // >=
  //
  IRTag_Cast,
  IRTag_Decl, // declaration
  IRTag_Ass,  // assignment
  // 
  IRTag_If,
  //
  IRTag_Block,
  // function
  IRTag_Fn,
  IRTag_FnParam,
};

#define TOKEN_KIND_RET_TAG(ID) case Tok##ID: return IRTag_##ID
// node string type
#define NodeInt     "I"
#define NodeDouble  "D"
#define NodeBool    "B"
#define NodeString  "S"
#define TagToStringCase(tag, str) \
  case IRTag_##tag : return str

inline std::string IRTag_to_string(IRNodeTag _tag)
{
  switch (_tag)
  {
    TagToStringCase(Id, "ID");
    TagToStringCase(Num, "Num");
    TagToStringCase(Add, "+");
    TagToStringCase(Sub, "-");
    TagToStringCase(Mul, "*");
    TagToStringCase(Div, "/");
    TagToStringCase(Eq, "==");
    TagToStringCase(Lt, "<");
    TagToStringCase(Le, "<=");
    TagToStringCase(Gt, ">");
    TagToStringCase(Ge, ">=");
  }
  throw_exception("tag to string error");
}

inline IRNodeTag token_kind_to_tag(TokenKind tk)
{
  switch (tk)
  {
    TOKEN_KIND_RET_TAG(Add);
    TOKEN_KIND_RET_TAG(Sub);
    TOKEN_KIND_RET_TAG(Mul);
    TOKEN_KIND_RET_TAG(Div);
    TOKEN_KIND_RET_TAG(Eq);
    TOKEN_KIND_RET_TAG(Lt);
    TOKEN_KIND_RET_TAG(Le);
    TOKEN_KIND_RET_TAG(Ge);
    TOKEN_KIND_RET_TAG(Gt);

  }
  throw_exception("token kind to tag error");
}

class IRNode;
class IR_Id;
class IR_Num;
class IR_Str;
class IR_BinOp;
class IR_Cast;
class IR_Decl;
class IR_Block;
class IR_If;
class IR_Fn;

template <typename _Ty>
using sptr_t = std::shared_ptr<_Ty> ;

#define Visitor_Methods_Decl()								\
  virtual IRNode* visit(IR_Id* id) = 0;				\
  virtual IRNode* visit(IR_Num* num) = 0;			\
  virtual IRNode* visit(IR_Str* str) = 0;			\
  virtual IRNode* visit(IR_BinOp* bin) = 0;		\
  virtual IRNode* visit(IR_Cast* cast) = 0;		\
  virtual IRNode* visit(IR_Decl* decl) = 0;		\
  virtual IRNode* visit(IR_If* stmt_if) = 0;	\
  virtual IRNode* visit(IR_Fn* stmt_fn) = 0;	\
  virtual IRNode* visit(IR_Block* block) = 0;

class IRVisitor
{
public:
  Visitor_Methods_Decl()
};

class Visitor
{
public:
  Visitor_Methods_Decl()
};

#define IR_Visitor_Impl_Decl()											\
  virtual IRNode* visit(IR_Id* id)  override;			  \
  virtual IRNode* visit(IR_Num* num) override ;		  \
  virtual IRNode* visit(IR_Str* str) override ;			\
  virtual IRNode* visit(IR_BinOp* _bin) override ;	\
  virtual IRNode* visit(IR_Cast* _cast) override ;	\
  virtual IRNode* visit(IR_Decl* _decl) override;		\
  virtual IRNode* visit(IR_If* stmt_if) override;		\
  virtual IRNode* visit(IR_Fn* stmt_fn) override;		\
  virtual IRNode* visit(IR_Block* block) override;

class IRNode
{
public:
  virtual IRNodeTag tag() { throw_exception("user shuold implement this fn"); return IRTag_Add; };
  virtual IRNode* accept(IRVisitor& visitor) { throw_exception("user shuold implement this fn"); return nullptr; };
  virtual IRNode* accept(Visitor& visitor) { throw_exception("user shuold implement this fn"); return nullptr; };
public:
  token_t		 token()		const	{ return _m_tok; }
  uint32_t start_loc()	const	{ return _m_begin_pos; }
  uint32_t end_loc()		const { return _m_end_pos; }
  void set_tok(token_t tok)				{ _m_tok = tok; }
  void set_end_loc(uint32_t _e)		{ _m_end_pos = _e; }
  void set_start_loc(uint32_t _s) { _m_begin_pos = _s; }

protected:
  IRNode() :IRNode(0, 0) {}
  IRNode(token_t _tok, uint32_t _e) 
    : IRNode(_tok, tok_foffset(_tok), _e) {};
  IRNode(token_t _tok, uint32_t _s, uint32_t _e) 
    : _m_tok(_tok), _m_begin_pos(_s), _m_end_pos(_e) {};

protected:
  uint32_t  _m_begin_pos;
  uint32_t  _m_end_pos;
  token_t		_m_tok;
};

class IR_String: protected Visitor
{
public:
  std::string stringify(IRNode* ir);
  void clear() { _m_ss.str(""); }
  std::string to_string() { return _m_ss.str(); }
private:
  IR_Visitor_Impl_Decl()
  inline void increase_indent() { m_indent++; }
  inline void indent_line () {
    _m_ss << " ";
  }
private:
  std::stringstream _m_ss;
  uint32_t 					m_indent;// current indent
};

// ************************** micro **************************
#define IR_Node_Accept_Visitor_Impl() \
public: \
  virtual IRNode* accept(Visitor& visitor) override  \
  { \
    return visitor.visit(this); \
  } 

#define IRNode_Visitor_Impl() \
public: \
  virtual IRNode* accept(IRVisitor& visitor) override {\
    return visitor.visit(this);\
  }

#define IRNode_Tag_Impl(Tag) \
public: \
  virtual IRNodeTag tag() override {\
    return Tag;\
  } 

#define IRNode_Impl(Tag) \
  IRNode_Tag_Impl(Tag) \
  IRNode_Visitor_Impl()
#define IrName(ir) IR_ ## ir

#define IrNode_Set_Method_Impl(ir, param) \
IrName(ir)* IrName(ir):: set_## param  (IRNode* in) { \
  if(in == nullptr )                                \
    printf("in is nullptr when set init\n");        \
  if(in == m_## param .get())                    \
    return this;																		\
  auto&& copy = new IrName(ir) (*this); 						\
  copy-> m_ ## param = sptr_t<IRNode>(in); 		\
  return copy;																			\
}
// ***********************************************************

// id node
class IR_Id: public IRNode
{
  IRNode_Impl(IRTag_Id)
  IR_Node_Accept_Visitor_Impl()
public:
  IR_Id(token_t tok, uint32_t _s)
    : IRNode(tok, _s) {}
  IR_Id(token_t tok, uint32_t _s, uint32_t _e)
    : IRNode(tok, _s, _e){}
};

// number node
class IR_Num : public IRNode
{
  friend class Parser;
  IRNode_Impl(IRTag_Num)
  IR_Node_Accept_Visitor_Impl()
public:
  IR_Num(token_t tok, uint32_t e): IRNode(tok, e), _m_num(){}
  IR_Num(token_t tok, uint32_t _s, uint32_t _e)
    : IRNode(tok, _s, _e), _m_num(){}
  bool is_int()		{ return token_is_kind(token(), TokInt); }
  bool is_float() { return token_is_kind(token(), TokFloat); }
  int			get_int() { return _m_num.i_value; }
  double	get_db()	{ return _m_num.db_value; }
private:
  union {
    double db_value;
    int		 i_value;
  } _m_num;
};

class IR_Str: public IRNode 
{
  friend class Parser;
  IRNode_Impl(IRTag_Str)
  IR_Node_Accept_Visitor_Impl()
public:
  IR_Str(token_t t, uint32_t e, FlString* str)
    : IRNode(t, e), m_str(str) {}
  FlString* str() const { return m_str; }
private:
  FlString* m_str;
};

class IR_Cast : public IRNode
{
  IRNode_Tag_Impl(IRTag_Cast)
  IR_Node_Accept_Visitor_Impl()
public:
  IR_Cast(token_t tok, uint32_t _e, IRNode* _sc, const unistr_t& _cst)
    : IRNode(tok, _e), _m_s(_sc), m_cast_target(_cst) {}
  IR_Cast(uint32_t _e, IRNode* _sc, const unistr_t& _cst)
    : IR_Cast(0, _e, _sc, _cst) {}
  const unistr_t& cast_to() { return m_cast_target; }
  IRNode* cast_from() { return _m_s.get(); }
  virtual IRNode* accept(IRVisitor& visitor) override
  {
    return visitor.visit(set_s(_m_s->accept(visitor)));
  }
protected:
  inline IR_Cast* set_s(IRNode* _s) {
    if (_s == _m_s.get()) return this;
    auto self = new IR_Cast(*this);
    self->_m_s = std::shared_ptr<IRNode>(_s);
    return self;
  }
private:
  sptr_t<IRNode> _m_s; // source node
  unistr_t m_cast_target;
};

// binary node
class IR_BinOp: public IRNode
{
  IR_Node_Accept_Visitor_Impl()
public:
  IR_BinOp(token_t tok, uint32_t _e, IRNodeTag _tag, IRNode* lhs, IRNode* rhs)
    : IRNode(tok, _e), _m_tag(_tag), _m_lhs(lhs), _m_rhs(rhs) {}
  IR_BinOp(token_t tok, uint32_t _e, IRNodeTag _tag)
    : IR_BinOp(tok, _e, _tag, nullptr, nullptr) {}
  virtual IRNodeTag tag() override { return _m_tag; }
  IRNode* lhs() const { return _m_lhs.get(); }
  IRNode* rhs() const { return _m_rhs.get(); }
  virtual IRNode* accept(IRVisitor& visitor) override;
protected:
  IR_BinOp* set_lhs(IRNode* _lhs);
  IR_BinOp* set_rhs(IRNode* _rhs);
private:
  sptr_t<IRNode> _m_lhs;
  sptr_t<IRNode> _m_rhs;
  IRNodeTag _m_tag;
};

class IR_Decl : public IRNode
{
  IRNode_Tag_Impl(IRTag_Decl)
  IR_Node_Accept_Visitor_Impl()
public:
  IR_Decl(token_t tok, uint32_t _e, IRNode* _init, const unistr_t& id)
    : IRNode(tok, _e), m_id(id), m_init(_init) {}
  virtual IRNode* accept(IRVisitor& visitor) override;
  const unistr_t& id() const { return m_id; }
  IRNode* init() const { return m_init.get(); }
protected:
  IR_Decl* set_init(IRNode* _init);
private:
  sptr_t<IRNode> m_init; // initial value
  unistr_t m_id;
};

class IR_Block : public IRNode
{
  IRNode_Tag_Impl(IRTag_Block)
  IR_Node_Accept_Visitor_Impl()
public:
  typedef std::vector<sptr_t<IRNode>> state_list_t;
  typedef state_list_t::iterator ls_iterator_t;
  IR_Block(token_t t, uint32_t _e, const state_list_t& ls)
    :IRNode(t,_e), m_state_list(ls) {}
  state_list_t& list() { return m_state_list; }
  virtual IRNode* accept(IRVisitor& visitor) override;
protected:
  inline IR_Block* proc_ls(IRVisitor& visitor)
  {
    state_list_t nls;
    bool changed = false;
    for (auto& it : m_state_list)
    {
      IRNode* itr = it->accept(visitor);
      if (itr != it.get()) {
        changed = true;
        nls.push_back(sptr_t<IRNode>(itr));
      }else {
        nls.push_back(it);
      }
    }
    if (!changed) return this;
    auto copy = new IR_Block(*this);
    copy->m_state_list = nls;
    return copy;
  }
private:
  state_list_t m_state_list;
};

class IR_If: public IRNode
{
  IRNode_Tag_Impl(IRTag_If)
  IR_Node_Accept_Visitor_Impl()
public:
  IR_If(token_t t, uint32_t end, IRNode* test, IRNode* success, IRNode* failed)
    : IRNode(t, end), m_test(test), m_success(success), m_failed(failed) { }
  IR_If(token_t t, uint32_t end, IRNode* test, IRNode* success)
    : IR_If(t, end, test, success, nullptr) {}

  inline IRNode* test() 		const { return m_test.get(); }
  inline IRNode* success() 	const { return m_success.get(); }
  inline IRNode* failed() 	const { return m_failed.get(); }
  inline bool has_failed() const { return m_failed.operator bool(); }
  virtual IRNode* accept(IRVisitor& visitor) override;

protected:
  IR_If* set_test(IRNode* test);
  IR_If* set_success(IRNode* success);
  IR_If* set_failed(IRNode* success);
private:
  sptr_t<IRNode> m_test;
  sptr_t<IRNode> m_success;
  sptr_t<IRNode> m_failed;
};

class IR_FnParam: public IRNode
{
  IRNode_Tag_Impl(IRTag_FnParam)
public:
  IR_FnParam(token_t t, uint32_t e, unistr_t id, unistr_t type_id)
    : IRNode(t, e), m_id(id), m_typeId(type_id) { }
public:
  unistr_t m_id;
  unistr_t m_typeId;
};

class IR_Fn: public IRNode
{
  IRNode_Tag_Impl(IRTag_Fn)
  IR_Node_Accept_Visitor_Impl()
public:
  inline bool is_named() const { return m_is_named; }
  inline const unistr_t& id() const { return m_id; }
  inline const unistr_t& return_type_id() const { return m_id; }
  inline IRNode* body() const { return m_body.get(); }
  inline std::vector<sptr_t<IR_FnParam>> params() const { return m_params; }
  inline void add_param(IR_FnParam* param) {  m_params.push_back(sptr_t<IR_FnParam>(param)); }

public:
  IR_Fn(token_t t, uint32_t e, unistr_t id, std::vector<sptr_t<IR_FnParam>> params, unistr_t return_type, IRNode* body)
    : IRNode(t, e), m_id(id), m_is_named(true), m_params(params), m_return_type(return_type), m_body(body) {}

  IR_Fn(token_t t, uint32_t e, std::vector<sptr_t<IR_FnParam>> params, unistr_t return_type, IRNode* body)
    : IRNode(t, e), m_id(""), m_is_named(false), m_params(params), m_return_type(return_type), m_body(body) {}

private:
  bool                            m_is_named;
  unistr_t                        m_id; // function name
  std::vector<sptr_t<IR_FnParam>> m_params;
  unistr_t                        m_return_type; // return type id
  sptr_t<IRNode>                  m_body;  // function body
};
