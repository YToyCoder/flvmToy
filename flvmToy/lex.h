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
#include "context.h"

#define BufSize 1024

class Lex 
{
public:
	Lex(const std::string& rFilename);
	~Lex();
	bool init(Context* c);

	// check if have token to read
	bool has_token();
	/*
	* return current token, fill token cache with next
	*/
	token_t next_token();
	// get current token
	token_t peek_token();
	// get the string value of token
	unistr_t token_string(token_t tok);
	// current position in file
	inline uint32_t cur_pos() const { return m_file_offset; }
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
		save_token_str(m_file_offset, next_char());
		//next_char(); // drop char
		return tok;
	}

	inline token_t one_or_two_token(TokenKind _one_char_kind, const std::pair<TokenKind,UChar>& kind_and_char)
	{
		unistr_t str;
		uint32_t pos_record = m_file_offset;
		str += next_char();
		if (get_cchar_from_uchar(peek_char()) == kind_and_char.second)
		{
			str += next_char();
			save_token_str(pos_record, str);
			return create_token(kind_and_char.first, pos_record, 2);
		}
		return create_token(_one_char_kind, pos_record, 1);
	}

	// buffer
	void try_ensure_buf();
	void fill_buffer();
	// char operations
	inline bool has_char() { return mBufLimit > mBufCursor || !u_feof(mFileHandle); }
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
	token_t proc_string_token();
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
	inline void save_token_str(uint32_t s, const unistr_t& str)
	{
		m_context->store_token_str(s, str);
	}
private:
	LexState mState;

	std::string mFilename;
	UFILE* mFileHandle;
	uint32_t	m_file_offset;

	UChar mbuf[BufSize];
	uint16_t mBufLimit;
	uint16_t mBufCursor;
	token_t mtok;

	Context* m_context;
};
