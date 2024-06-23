/*****************************************************************/ /**
 * @file   colt_type_buffer.h
 * @brief  Contains TypeBuffer, responsible for storing types.
 * When compiling, the TypeBuffer is shared by all the files
 * representing the program.
 *
 * @author RPC
 * @date   March 2024
 *********************************************************************/
#ifndef HG_COLTC_TYPE_BUFFER
#define HG_COLTC_TYPE_BUFFER

#include "colt_type.h"
#include "structs/set.h"
#include "colt_type_token.h"

namespace clt::lng
{
  /// @brief Class responsible of storing types.
  class TypeBuffer
  {
    /// @brief Set of types
    IndexedSet<TypeVariant> type_map{};
    /// @brief The set of function payloads (used by function types)
    IndexedSet<FnTypePayload> fn_payloads{};
    /// @brief The type names
    mutable Vector<String> type_names;

    /// @brief Returns the next token to save
    /// @return The token to save
    constexpr TypeToken create_token(size_t sz) const noexcept
    {
      assert_true("Integer overflow!", sz <= std::numeric_limits<u32>::max());
      return TypeToken(static_cast<u32>(sz));
    }

  public:
    /// @brief Default constructor
    TypeBuffer() noexcept = default;

    StringView type_name(const TypeVariant& variant) const noexcept;
    StringView type_name(TypeToken variant) const noexcept;

    /// @brief Saves a type and return its index number
    /// @param variant The type to save
    /// @return The TypeToken representing the type
    TypeToken add_type(const TypeVariant& variant) noexcept
    {
      auto [pair, insert] = type_map.insert(variant);
      return create_token(pair);
    }

    /// @brief Returns an error type
    /// @return Error Type
    TypeToken error_type() noexcept
    {
      return add_type(make_coltc_type<ErrorType>());
    }

    /// @brief Returns a void type
    /// @return Void Type
    TypeToken void_type() noexcept { return add_type(make_coltc_type<VoidType>()); }

    /// @brief Saves a built-in type
    /// @param id The type ID
    /// @return The TypeToken representing the type
    TypeToken add_builtin(BuiltinID id) noexcept
    {
      return add_type(ColtBuiltinTypeTable[static_cast<u8>(id)]);
    }

    /// @brief Saves a pointer to a type
    /// @param to The type pointed to
    /// @return The TypeToken representing the pointer
    TypeToken add_ptr(TypeToken to) noexcept
    {
      return add_type(make_coltc_type<PtrType>(to));
    }

    /// @brief Saves a mutable pointer to a type
    /// @param to The type pointed to
    /// @return The TypeToken representing the mutable pointer
    TypeToken add_mut_ptr(TypeToken to) noexcept
    {
      return add_type(make_coltc_type<MutPtrType>(to));
    }

    /// @brief Saves a pointer to a type
    /// @param to The type pointed to
    /// @return The TypeToken representing the pointer
    TypeToken add_opaque_ptr() noexcept
    {
      return add_type(make_coltc_type<OpaquePtrType>());
    }

    /// @brief Saves a mutable pointer to a type
    /// @param to The type pointed to
    /// @return The TypeToken representing the mutable pointer
    TypeToken add_mut_opaque_ptr() noexcept
    {
      return add_type(make_coltc_type<MutOpaquePtrType>());
    }

    /// @brief Creates a function type
    /// @param return_type The return type of the function
    /// @param arguments_type The arguments type of the function
    /// @param is_c_variadic True if the function uses C variadic arguments
    /// @return The TypeToken representing the function
    TypeToken add_fn(
        TypeToken return_type, Vector<FnTypeArgument>&& arguments_type,
        bool is_c_variadic = false) noexcept
    {
      auto [pair, insert] = fn_payloads.insert(
          {is_c_variadic, return_type, std::move(arguments_type)});
      assert_true(
          "Integer overflow detected!", pair <= std::numeric_limits<u32>::max());
      return add_type(make_coltc_type<FnType>(static_cast<u32>(pair)));
    }

    /// @brief Get a type using its token.
    /// The reference is valid as long as add_type was not called!
    /// @param tkn The token whose type to return
    /// @return The type represented by 'tkn'
    const TypeVariant& type(TypeToken tkn) const noexcept
    {
      return type_map.internal_list()[tkn.getID()];
    }
  };
} // namespace clt::lng

#endif // !HG_COLTC_TYPE_BUFFER
