#ifndef HG_COLT_TOKEN_HELPER
#define HG_COLT_TOKEN_HELPER

/// @brief Expands to default copy/move constructor/assignment operator
#define MAKE_DEFAULT_COPY_AND_MOVE_FOR(type) \
  constexpr type(type&&) noexcept = default; \
  constexpr type(const type&) noexcept = default; \
  constexpr type& operator=(type&&) noexcept = default; \
  constexpr type& operator=(const type&) noexcept = default;

#define IMPL_CREATE_TOKEN_FRIEND_CLASS(a) friend class a;

#define CREATE_TOKEN_TYPE(name, type, max_value, friend1, ...) \
  struct name \
  { \
    type index; \
    constexpr name(type index) noexcept: index(index) {} \
    MAKE_DEFAULT_COPY_AND_MOVE_FOR(name); \
  public: constexpr type getID() const noexcept { return index; } \
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
  class OptTok
  {
    static_assert(T::MAX_VALUE < std::numeric_limits<typename T::storage_t>::max(),
      "Use Option<> as OptTok cannot take advantage of stored data!");

    using ty = typename T::storage_t;

    /// @brief The index
    ty index;

    static constexpr ty INVALID = T::MAX_VALUE + 1;

  public:
    OptTok() = delete;
    
    /// @brief Constructor
    /// @param tkn The token whose content to store
    constexpr OptTok(T tkn) noexcept
      : index(tkn.getID()) {}
    /// @brief Constructor
    constexpr OptTok(decltype(None)) noexcept
      : index(T::MAX_VALUE + 1) {}

    MAKE_DEFAULT_COPY_AND_MOVE_FOR(OptTok);

    /// @brief Check if the optional does not contain a value
    /// @return True if the optional is empty
    constexpr bool isNone() const noexcept { return index == INVALID; }
    /// @brief Check if the optional contains a value
    /// @return True if the optional is not empty
    constexpr bool isValue() const noexcept { return !isNone; }

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