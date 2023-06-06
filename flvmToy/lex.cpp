#include <stdio.h>
#include <iostream>
#include "lex.h"
#include "unicode/ustream.h"


Lex::Lex(const std::string& rFilename)
	: mFilename(rFilename), mState(LexState_Created), mFileHandle(nullptr),
	mBufLimit(0), mBufCursor(0), mFilePosition({ 0,0 }), mtok(InvalidTok)
{
}

bool Lex::init() {
	mFileHandle = u_fopen(mFilename.c_str(), "r", NULL, NULL);
	if (mFileHandle == NULL) return false;
	mBufCursor = 0;
	mBufLimit = 0;
	mState = LexState_Init;

	// read the first token
	next_token();
	return true;
}

bool Lex::check_state() const {
	if (mState == LexState_Init) { return true; }
	if (mState == LexState_Created) {
		printf("Lex haven't been initialized !!");
	}
	if (mState == LexState_Closed) {
		printf("Lex have been closed !!");
	}
	return false;
}

// token 
bool Lex::has_token() const {
	return TokIsValid(mtok) || has_char();
}

token_t Lex::next_token() {
	check_state();
	token_t tok = mtok;
	mtok = fetch_token();
	return tok;
}

token_t Lex::peek_token() {
	return mtok;
}
void Lex::ignore_space() {
	while (has_char() && u_isJavaSpaceChar(codepoint())) {
		next_char();
	}
}
// todo
token_t Lex::fetch_token() {
	ignore_space();
	UChar32 cp = codepoint();
	UChar uc = peek_char();
	if (is_eol(uc)) {
		return one_char_token(TokEol);
	}
	else if (is_alpha(cp) || is_underscore(uc)) {
		return alphabetic_start_token();
	}
	else {
		// operator : + - * /
		switch (get_cchar_from_uchar(uc)) {
			case '+': return one_char_token(TokAdd);
			case '-': return one_char_token(TokSub);
			case '*': return one_char_token(TokMul);
			case '/': return one_char_token(TokDiv);
		}
	}
	// need throw exception 
	// reach error
#if 1
	printf("error char is %c\n", (char)uc);
#endif
	throw std::exception("not support token type, start with");
}

void Lex::fill_buffer(){
	mBufLimit = u_file_read(mbuf, BufSize, mFileHandle);
	mBufCursor = 0;
}

// char operations
UChar Lex::next_char() {
	try_ensure_buf();
	// check buffer has content
	// the file may have been enter eof,
	// if does not contain the char , 
	// there will be need throw an exception
	if (mBufLimit == 0) {
		// do throw 
		throw std::exception("encounter eof while try to get char!");
	}
	if (is_eol(peek_char())) {
		mFilePosition.next_line();
	}
	else {
		mFilePosition.next_col();
	}
	return mbuf[mBufCursor++];
}

void Lex::try_ensure_buf() {
	if (mBufCursor >= mBufLimit) {
		fill_buffer();
	}
}

token_t Lex::alphabetic_start_token() {
	Position token_start = mFilePosition;
	UStr str;
	UChar32 cp;
	UChar uc;
	auto char_is_id_iner = [this](UChar32 cp, UChar uc) -> bool {
		return is_alpha(cp) || is_underscore(uc) || is_numeric(cp);
	};
	do {
		cp = codepoint();
		uc = next_char();
		str.append(cp);
	} while (has_char() && char_is_id_iner(codepoint(), peek_char()));
#if 0
	std::cout << "read token str " << str << std::endl;
#endif
	Position token_end = mFilePosition;
	return create_token(TokId, token_start.mY, token_start.mX, token_end.mX);
}