#pragma once
#include <string>
#include <stdint.h>

#include "common.h"

#define TOK_TEST

enum TokenKind : uint8_t
{
	TokId  = 0,
	TokInt,
	TokFloat,
	// operand 
	TokAdd , // +
	TokSub , // -
	TokMul , // *
	TokDiv , // /
	TokLParent , // (
	TokRParent ,  // )
	// control
	TokEol, // end of line => \n
	TokEof, // end of file
	// not valid token
	TokNan,
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
typedef uint64_t token_t;

#define InvalidTok (0x0100000000000000)
#define TokIsValid(t) (t != InvalidTok)

std::string tk_to_str(token_t tk);
token_t create_token(TokenKind _kind, uint16_t row, uint16_t start_col, uint16_t end_col);
bool token_is_kind(token_t tok, TokenKind _kind); 
uint16_t token_row(token_t tok);
uint16_t token_scol(token_t tok);
uint16_t token_ecol(token_t tok); 
std::string token_to_str(token_t tok);

#ifdef TOK_TEST
void tok_test1();
#endif
