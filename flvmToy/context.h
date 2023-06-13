#pragma once
#include "token.h"
#include "unicode/ustream.h"
#include <map>

class Context
{
public:
	void store_token_str(token_t t, const unistr_t& str)
	{
		uint32_t s = tok_foffset(t);
		m_tok_str.insert(std::make_pair(s, str));
	}

	void store_token_str(uint32_t s, const unistr_t& str)
	{
		m_tok_str.insert(std::make_pair(s, str));
	}

	bool find_tok_str(token_t t, unistr_t& out)
	{
		auto it = m_tok_str.find(tok_foffset(t));
		if (it == m_tok_str.end())
		{
			return false;
		}
		out = it->second;
		std::cout << "find string : " << out
			<< " for " << token_to_str(t) << std::endl;
		return true;
	}

	void print()
	{
		for (auto& it : m_tok_str)
		{
			std::cout << std::to_string(it.first) << " " << it.second << " ";
		}
		std::cout << std::endl;
	}
private:
	std::map<uint32_t, unistr_t> m_tok_str;
};


