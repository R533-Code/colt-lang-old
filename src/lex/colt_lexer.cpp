#include "colt_lexer.h"

namespace clt::lng
{
#ifdef COLT_DEBUG
  std::atomic<u32> TokenBuffer::ID_GENERATOR{};
#endif // COLT_DEBUG

  TokenBuffer Lex(ErrorReporter_t reporter, StringView to_parse) noexcept
  {
    TokenBuffer buffer;
    CreateLines(to_parse, buffer.lines);

    return buffer;
  }
  
  void CreateLines(StringView strv, FlatList<StringView, 256>& buffer) noexcept
  {
    const auto front = strv.data();
    const char* const text = strv.data();
    const i64 size = strv.size();
    i64 start = 0;
    while (auto nl = static_cast<const char*>(memchr(&text[start], '\n', size - start)))
    {
      i64 nl_index = nl - text;
      buffer.push_back(StringView{ front + start, static_cast<size_t>(nl_index - start) });
      start = nl_index + 1;
    }
    // The last line ends at the end of the file.
    buffer.push_back(StringView{ front + start, static_cast<size_t>(size - start) });

    // Add an extra blank line so that we never need to handle the special case
    // of being on the last line inside the lexer and needing to not increment
    // to the next line.
    if (start != size)
      buffer.push_back(StringView{ front + start, 0 });
  }
}



