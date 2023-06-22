#include <stdio.h>
#include <iostream>
#include "lex.h"
#include "unicode/ustream.h"


Lex::Lex(const std::string& rFilename)
  : mFilename(rFilename), mState(LexState_Created), mFileHandle(nullptr),
  mBufLimit(0), mBufCursor(0), mtok(InvalidTok)
{
}

Lex::~Lex()
{
  u_fclose(mFileHandle);
}

bool Lex::init(Context* c) {
  mFileHandle = u_fopen(mFilename.c_str(), "r", NULL, NULL);
  if (mFileHandle == NULL) return false;
  mBufCursor = 0;
  mBufLimit = 0;
  m_context = c;
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
bool Lex::has_token() {
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
  if (!has_char()) {
    return InvalidTok;
  }
  UChar32 cp = codepoint();
  UChar uc = peek_char();
  if (is_eol(uc)) {
    return one_char_token(TokEol);
  }
  else if (is_eof(uc)) {
    return one_char_token(TokEof);
  }
  else if (is_alpha(cp) || is_underscore(uc)) {
    return alphabetic_start_token();
  }
  else if (is_numeric(cp)) {
    return numeric_token();
  }
  else {
    // operator : + - * /
    switch (get_cchar_from_uchar(uc)) {
      case '+': return one_char_token(TokAdd);
      case '-': return one_char_token(TokSub);
      case '*': return one_char_token(TokMul);
      case '/': return one_char_token(TokDiv);
      case '=': return one_or_two_token(TokAssign, std::make_pair(TokEq,'='));
      case '(': return one_char_token(TokLParent);
      case ')': return one_char_token(TokRParent);
      case '{': return one_char_token(TokLBrace);
      case '}': return one_char_token(TokRBrace);
      case '<': return one_or_two_token(TokLt, std::make_pair(TokLe, '='));
      case '>': return one_or_two_token(TokGt, std::make_pair(TokGe, '='));
      case '"': return proc_string_token();
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
  m_file_offset++;
  return mbuf[mBufCursor++];
}

void Lex::try_ensure_buf() {
  if (mBufCursor >= mBufLimit && !u_feof(mFileHandle)) {
    fill_buffer();
  }
}
token_t Lex::numeric_token() {
  uint32_t token_start = m_file_offset;
  unistr_t str;
  auto proc_num = [this](unistr_t& str) {
    UChar32 cp;
    UChar uc;
    do {
      cp = codepoint();
      uc = next_char();
      str += uc;
    } while (has_char() && is_numeric(codepoint()));
  };
  proc_num(str);
  if (has_char() && is_dot(peek_char())) {
    str += next_char(); // drop dot
    proc_num(str);
    save_token_str(token_start, str);
    return create_token(TokFloat, token_start, m_file_offset - token_start);
  }
  save_token_str(token_start, str);
  return create_token(TokInt, token_start, m_file_offset - token_start);
}

token_t Lex::alphabetic_start_token() {
  uint32_t token_start = m_file_offset;
  unistr_t str;
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
  save_token_str(token_start, str);
  TokenKind tk ;
  if (str == "let") tk =TokLet;
  else if(str == "if") tk = TokIf;
  else if(str == "else") tk = TokElse;
  else tk = TokId;
  return create_token(tk, token_start, m_file_offset - token_start);
}

token_t Lex::proc_string_token()
{
  uint32_t token_start = m_file_offset;
  unistr_t str;
  UChar32 cp;
  UChar uc;
  next_char(); // consume " char
  do {
    cp = codepoint();
    uc = next_char();
    str += uc;
  }while(
    has_char() && 
    !(get_cchar_from_uchar(peek_char()) == '"' || is_eol(peek_char())));

  save_token_str(token_start, str);
  std::cout << str << std::endl;
  if(has_char() && get_cchar_from_uchar(peek_char()) == '"') {
    next_char();
  }
  return create_token(TokStr, token_start, m_file_offset - token_start);
}

unistr_t Lex::token_string(token_t tok)
{
  unistr_t str;
  if(m_context->find_tok_str(tok, str))
    return str;
  std::cout <<  "not find str for token " << token_to_str(tok) << std::endl;
  m_context->print();
  throw std::exception("not find str for token");
}
