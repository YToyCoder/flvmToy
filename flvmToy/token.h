#pragma once
#include <string>
#include <stdint.h>

#include "common.h"

#define TOK_TEST

enum TokenKind : uint8_t
{
  // not valid token
  TokNan = 0,
  TokId,
  TokInt,
  TokFloat,
  TokStr,
  // operand 
  TokAdd , // +
  TokSub , // -
  TokMul , // *
  TokDiv , // /
  TokLParent , // (
  TokRParent ,  // )
  TokLBrace, // {
  TokRBrace, // }
  // declaration
  TokLet, // let
  TokAssign, // =
  // 
  TokEq, // ==
  TokLt, // <
  TokLe, // <=
  TokGt, // >
  TokGe, // >=
  //
  TokColon,  // :
  TokComma,  // ,
  // control
  TokIf,
  TokElse,
  TokFn,  // function
  TokEol, // end of line => \n
  TokEof, // end of file
};

enum TokendFlag : uint8_t
{
  TokFlagISTok = 0,
  TokFlagNOTTok = 1
};
// position: column row 16 * 2 = 32
// token --> unsigned long long
// <- 08 -> | <- 08 -> | <-- 16 --> | <-- 16 --> | <-- 16 -->
// |	  	  | tok-kind |     row    | start: col | end: row |
// | <- 08 -> | <- 08 -> | <-- 16 --> | <-- 16 --> | <-- 16 -->
// | tok-kind |		offset |		file--offset				 |					 |
typedef uint64_t token_t;

#define InvalidTok (0x0000000000000000)
#define TokIsValid(t) (!token_is_kind(t, TokNan))

std::string tk_to_str(token_t tk);
token_t create_token(TokenKind _kind, uint32_t fileOffset, uint32_t token_offset);
bool token_is_kind(token_t tok, TokenKind _kind); 
TokenKind token_kind(token_t tok);
uint32_t tok_foffset(token_t tok);
uint32_t tok_end(token_t tok);
uint32_t tok_len(token_t tok);
std::string token_to_str(token_t tok);

inline bool token_is_operator(token_t tok)
{
  switch (token_kind(tok))
  {
  case TokAdd:
  case TokDiv:
  case TokMul:
  case TokSub:
    return true;
  }
  return false;
}

inline bool token_is_comp_operator(token_t tok)
{
  switch(token_kind(tok))
  {
    case TokEq:
    case TokLe:
    case TokLt:
    case TokGe:
    case TokGt:	return true;
    default: 		return false;
  }
}
