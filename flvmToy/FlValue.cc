#include "FlValue.h"

FlString* StringPool::of_string(const UChar* cs, size_t len)
{
  uint64_t hscode = hash_cstr(reinterpret_cast<const char*>(cs), len * 2);
  auto&& it = m_string_map.find(hscode);
  if(it != m_string_map.end()) return it->second;
  FlString* new_str = new FlString(cs, len);
  m_string_map.insert(std::make_pair(hscode,  new_str));
  return new_str;
}

FlString* StringPool::of_string(FlString* str)
{
  uint64_t hash_code = str->hash();
  auto&& it = m_string_map.find(hash_code);
  if(it != m_string_map.end()) return it->second;
  m_string_map.insert(std::make_pair(hash_code, str));
  return str;
}