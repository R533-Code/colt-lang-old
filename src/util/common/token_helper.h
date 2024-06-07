/*****************************************************************//**
 * @file   token_helper.h
 * @brief  Contains macros to simplify creating tokens.
 * 
 * @author RPC
 * @date   April 2024
 *********************************************************************/
#ifndef HG_COLT_TOKEN_HELPER
#define HG_COLT_TOKEN_HELPER

/// @brief Expands to default copy/move constructor/assignment operator
#define MAKE_DEFAULT_COPY_AND_MOVE_FOR(type) \
  constexpr type(type&&) noexcept = default; \
  constexpr type(const type&) noexcept = default; \
  constexpr type& operator=(type&&) noexcept = default; \
  constexpr type& operator=(const type&) noexcept = default;

/// @brief Expands to delete copy/move constructor/assignment operator
#define MAKE_DELETE_COPY_AND_MOVE_FOR(type) \
  type(type&&) = delete; \
  type(const type&) = delete; \
  type& operator=(type&&) = delete; \
  type& operator=(const type&) = delete;

#define IMPL_CREATE_TOKEN_FRIEND_CLASS(a) friend class a;

/// @brief Creates a token class
#define CREATE_TOKEN_TYPE(name, type, max_value, friend1, ...) \
  struct name \
  { \
    type index; \
    constexpr name(type index) noexcept: index(index) {} \
  public: constexpr type getID() const noexcept { return index; } \
    MAKE_DEFAULT_COPY_AND_MOVE_FOR(name); \
    COLT_FOR_EACH(IMPL_CREATE_TOKEN_FRIEND_CLASS, friend1, __VA_ARGS__) \
    constexpr bool operator==(const name&) const noexcept = default; \
    using storage_t = type; \
    static constexpr type MAX_VALUE = max_value; \
    template<TokenType T> \
    friend class OptTok; \
  };

namespace clt::lng
{
  template<typename T>
  /// @brief Check if a type is a token (usually created through macro CREATE_TOKEN_TYPE).
  concept TokenType = std::same_as<std::remove_cv_t<typename T::storage_t>, std::remove_cv_t<decltype(T::MAX_VALUE)>>
    && std::same_as<std::remove_cv_t<typename T::storage_t>, std::remove_cv_t<decltype(std::declval<T>().getID())>>;

  template<TokenType T>
  /// @brief Represents an optional token
  class OptTok
  {
    static_assert(T::MAX_VALUE < std::numeric_limits<typename T::storage_t>::max(),
      "Use Option<> as OptTok cannot take advantage of stored data!");
    /// @brief The storage type
    using ty = typename T::storage_t;

    /// @brief The index
    ty index;

    /// @brief The invalid value used to represent None
    static constexpr ty INVALID = T::MAX_VALUE + 1;

  public:
    OptTok() = delete;
    
    /// @brief Constructor
    /// @param tkn The token whose content to store
    constexpr OptTok(T tkn) noexcept
      : index(tkn.getID()) {}
    /// @brief Constructor
    constexpr OptTok(decltype(None)) noexcept
      : index(INVALID) {}

    MAKE_DEFAULT_COPY_AND_MOVE_FOR(OptTok);

    /// @brief Assigns a value to the optional
    /// @param t The value to assign
    /// @return this
    constexpr OptTok& operator=(T t) noexcept { index = t.getID(); return *this; }
    /// @brief Clears the optional
    /// @param  None
    /// @return this
    constexpr OptTok& operator=(decltype(None)) noexcept { index = INVALID; return *this; }

    /// @brief Check if the optional does not contain a value
    /// @return True if the optional is empty
    constexpr bool isNone() const noexcept { return index == INVALID; }
    /// @brief Check if the optional contains a value
    /// @return True if the optional is not empty
    constexpr bool isValue() const noexcept { return !isNone(); }

    /// @brief Returns the value, isValue must be true.
    /// @return The value
    constexpr T getValue() const noexcept
    {
      assert_true("OptTok was empty!", isValue());
      return T{ index };
    }
  };
}

#endif //HG_COLT_TOKEN_HELPER