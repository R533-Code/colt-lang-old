#ifndef HG_COLTC_TYPE_TOKEN
#define HG_COLTC_TYPE_TOKEN

#include "util/types.h"

namespace clt::lng
{
  class TypeBuffer;

  /// @brief Represent a type through its index
  class TypeToken
  {
    u32 type_index;

    friend class TypeBuffer;

#ifdef COLT_DEBUG
    /// @brief On debug, we store the ID of the TokenBuffer owning the Token
    u32 buffer_id;

    constexpr TypeToken(u32 id, u32 buffer_id) noexcept
      : type_index(id), buffer_id(buffer_id) {}

#else
    constexpr TypeToken(u32 id) noexcept
      : type_index(id) {}
#endif // COLT_DEBUG
  
  public:
    TypeToken() = delete;
    constexpr TypeToken(TypeToken&&) noexcept = default;
    constexpr TypeToken(const TypeToken&) noexcept = default;
    constexpr TypeToken& operator=(TypeToken&&) noexcept = default;
    constexpr TypeToken& operator=(const TypeToken&) noexcept = default;

    constexpr bool operator==(const TypeToken&) const noexcept = default;

    constexpr u32 getID() const noexcept { return type_index; }
  };
}

#endif // !HG_COLTC_TYPE_TOKEN
