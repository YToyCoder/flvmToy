#pragma once
#include <fstream>
#include <string>
#include <cstdint>
#include <map>

#include "unicode/utf16.h"
#include "unicode/uchar.h"
#include "unicode/ustdio.h"
#include "unicode/unistr.h"

#include "token.h"

#define BufSize 1024

U_NAMESPACE_USE;

class Lex 
{
public:
	typedef UnicodeString UStr;
public:
	Lex(const std::string& rFilename);
	~Lex();
	bool init();

	// check if have token to read
	bool has_token();
	/*
	* return current token, fill token cache with next
	*/
	token_t next_token();
	// get current token
	token_t peek_token();
	// get the string value of token
	UStr token_string(token_t tok);
protected:
#define LexBufCapability 100
	enum LexState {
		LexState_Created = 0,
		LexState_Init ,
		LexState_Closed
	};
	// checking state
	inline bool check_state() const ;
	inline void ignore_space();
	inline token_t fetch_token();
	inline token_t one_char_token(TokenKind _kind) 
	{ 
		token_t tok = create_token(_kind, m_file_offset, 1);
		next_char(); // drop char
		return tok;
	}
	// buffer
	void try_ensure_buf();
	void fill_buffer();
	// char operations
	inline bool has_char() { 
		return mBufLimit > mBufCursor || !u_feof(mFileHandle); 
	}
	inline UChar next_char(); 
	inline UChar peek_char() { try_ensure_buf(); return mbuf[mBufCursor]; }
	inline UChar32 codepoint() { 
		try_ensure_buf();
		UChar32 cp;
		U16_GET(mbuf, 0, mBufCursor, mBufLimit, cp);
		return cp;
	}
	// string operation
	token_t alphabetic_start_token();
	token_t numeric_token();
protected:
	inline char get_cchar_from_uchar(UChar uc) { return uc & 0x007F; }
	// end of line 
	inline bool is_eol(UChar c)						{ return get_cchar_from_uchar(c) == '\n'; }
	inline bool is_eol(UChar32 codepoint) { return u_charType(codepoint) == U_CONTROL_CHAR; }
	inline bool is_eof(UChar uc)					{ return uc == U_EOF;  }
	inline bool is_alpha(UChar32 codepoint) { return u_isalpha(codepoint); }
	inline bool is_underscore(UChar c)		{ return (char)(c & 0x007F) == '_'; }
	inline bool is_numeric(UChar32 cp)		{ return u_isalnum(cp); }
	inline bool is_dot(UChar uc)					{ return get_cchar_from_uchar(uc) == '.'; }
private:
	LexState mState;

	std::string mFilename;
	UFILE* mFileHandle;
	uint32_t	m_file_offset;

	UChar mbuf[BufSize];
	uint16_t mBufLimit;
	uint16_t mBufCursor;
	token_t mtok;
	
	std::map<uint32_t, UStr> m_str;
};
