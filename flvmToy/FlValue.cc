#include "FlValue.h"
#include "unicode/ustream.h"
#include "unicode/ustdio.h"
#include <iomanip>

// vm obj

std::ostream& operator<<(std::ostream& stream, const FlTagValue& value)
{
      switch(value.tag()){
        case FlTagValue::IntTag:    stream << value.union_v()._int; break;
        case FlTagValue::DoubleTag: stream << std::setprecision(10) <<(FlDouble) value.union_v()._double; break;
        case FlTagValue::StrTag:    
        {
          FlString* string = (FlString*)value.union_v()._obj;
          u_printf_u(string->chars());
        }
          break;
        case FlTagValue::ObjTag:    stream << "obj"; break;
        case FlTagValue::BoolTag:   stream << value.union_v()._bool; break;
        default:                    stream << "nil"; break;
      };

  return stream;
}

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