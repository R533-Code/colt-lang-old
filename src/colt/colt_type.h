#ifndef HG_COLTC_TYPE
#define HG_COLTC_TYPE

#include "util/types.h"
#include "meta/meta_type_list.h"
#include "colt_builtin_id.h"

DECLARE_ENUM_WITH_TYPE(u8, clt::lng, TypeID,
  TYPE_ERROR, TYPE_BUILTIN, TYPE_VOID,
  TYPE_PTR, TYPE_MUT_PTR, TYPE_OPTR, TYPE_MUT_OPTR
);

namespace clt::lng
{
  using TypeToken = u32;

  template<typename T>
  concept ColtType = std::equality_comparable<T> && requires (T a)
  {
    { a.classof() } -> std::same_as<TypeID>;
  };

  template<typename T>
  constexpr TypeID TypeToTypeID() noexcept;

  class TypeBase
  {
    TypeID type_id;
  
  public:
    constexpr TypeBase() noexcept = delete;
    constexpr TypeBase(TypeID id) noexcept
      : type_id(id) {}
    constexpr TypeBase(TypeBase&&) noexcept = default;
    constexpr TypeBase& operator=(TypeBase&&) noexcept = default;

    constexpr TypeID classof() const noexcept { return type_id; }
  };

#define CREATE_TYPE(name) class name final : public TypeBase\
                          { \
                          public: \
                            constexpr name() noexcept : TypeBase(TypeToTypeID<name>()) {} \
                            constexpr name(name&&) noexcept = default; \
                            constexpr name& operator=(name&&) noexcept = default; \
                            constexpr bool operator==(const name&) const { return true; } \
                          }

  // Create the empty types.
  // These types have the default constructor,
  // and an equality comparison operator.

  CREATE_TYPE(ErrorType);
  CREATE_TYPE(VoidType);
  CREATE_TYPE(OpaquePtrType);
  CREATE_TYPE(MutOpaquePtrType);

#undef CREATE_TYPE

  class BuiltinType
    final : public TypeBase
  {
    BuiltinID type_id;

  public:
    constexpr BuiltinType(BuiltinID id) noexcept
      : TypeBase(TypeToTypeID<BuiltinType>()), type_id(id) {}
    constexpr BuiltinType() noexcept = delete;
    constexpr BuiltinType(BuiltinType&&) noexcept = default;
    constexpr BuiltinType& operator=(BuiltinType&&) noexcept = default;

    constexpr bool operator==(const BuiltinType&) const = default;

    constexpr BuiltinID typeID() const noexcept { return type_id; }
  };

  class PtrType
    final : public TypeBase
  {
    TypeToken type_id;

  public:
    constexpr PtrType(TypeToken id) noexcept
      : TypeBase(TypeToTypeID<PtrType>()), type_id(id) {}
    constexpr PtrType(PtrType&&) noexcept = default;
    constexpr PtrType& operator=(PtrType&&) noexcept = default;

    constexpr bool operator==(const PtrType&) const = default;

    constexpr TypeToken getPointingTo() const noexcept { return type_id; }
  };
  
  class MutPtrType
    final : public TypeBase
  {
    TypeToken type_id;

  public:
    constexpr MutPtrType(TypeToken id) noexcept
      : TypeBase(TypeToTypeID<MutPtrType>()), type_id(id) {}
    constexpr MutPtrType(MutPtrType&&) noexcept = default;
    constexpr MutPtrType& operator=(MutPtrType&&) noexcept = default;

    constexpr bool operator==(const MutPtrType&) const = default;

    constexpr TypeToken getPointingTo() const noexcept { return type_id; }
  };

  using ColtTypeList = meta::type_list<
    ErrorType, VoidType, OpaquePtrType, MutOpaquePtrType,
    BuiltinType, PtrType, MutPtrType>;

  static_assert(ColtTypeList::size == reflect<TypeID>::count(),
    "ColtTypeList and TypeID count must be equal!");

  class TypeVariant
  {
    union AllTypes
    {
      ErrorType        error_type;
      VoidType         void_type;
      OpaquePtrType    opaque_type;
      MutOpaquePtrType mut_opaque_type;
      BuiltinType      builtin_type;
      PtrType          ptr_type;
      MutPtrType       mut_ptr_type;
    };

    AllTypes variant;

  public:
    template<ColtType Type, typename... Args>
    constexpr TypeVariant(Args&&... args) noexcept(std::is_nothrow_constructible_v<Type, Args...>)
    {
      new(variant) Type(std::forward<Args>(args)...);
    }

    /// @brief Returns the type ID of the current type
    /// @return The ID of the current type
    constexpr TypeID getTypeID() const noexcept
    {
      // We cannot read the value through any of the members
      // because of UB.
      // The memcpy is optimized away by the compiler.
      TypeID id;
      if (std::is_constant_evaluated())
        id = std::bit_cast<TypeID>(variant.error_type);
      else
        std::memcpy(&id, &variant, sizeof(TypeID));
      return id;
    }

    /// @brief Check if the type is a mutable pointer
    /// @return 
    constexpr bool isMutPtr() const noexcept
    {
      return getTypeID() == TypeID::TYPE_MUT_OPTR || getTypeID() == TypeID::TYPE_MUT_PTR;
    }

    constexpr bool isPtr() const noexcept
    {
      return getTypeID() == TypeID::TYPE_OPTR || getTypeID() == TypeID::TYPE_PTR;
    }

    /// @brief Check if the current type is built-in and 'check' its ID.
    /// The check is performed with the built-in ID only if the current type
    /// is built-in.
    /// @param check The function to check for if built-in
    /// @return True only if built-in and 'check' returns true
    constexpr bool isBuiltinAnd(BuilinTypeCheck_t check) const noexcept
    {
      return getTypeID() == TypeID::TYPE_BUILTIN
        && check(variant.builtin_type.typeID());
    }

    /// @brief Check if the current type is void
    constexpr bool isVoid()    const noexcept { return getTypeID() == TypeID::TYPE_VOID; }
    /// @brief Check if the current type is an error
    constexpr bool isError()   const noexcept { return getTypeID() == TypeID::TYPE_ERROR; }
    /// @brief Check if the current type is a built-in type
    constexpr bool isBuiltin() const noexcept { return getTypeID() == TypeID::TYPE_BUILTIN; }
  };

  template<ColtType T>
  constexpr TypeID TypeToTypeID() noexcept
  {
    using enum TypeID;

    if constexpr (std::same_as<T, ErrorType>)
      return TYPE_ERROR;
    if constexpr (std::same_as<T, BuiltinType>)
      return TYPE_BUILTIN;
    if constexpr (std::same_as<T, VoidType>)
      return TYPE_VOID;
    if constexpr (std::same_as<T, OpaquePtrType>)
      return TYPE_OPTR;
    if constexpr (std::same_as<T, MutOpaquePtrType>)
      return TYPE_MUT_OPTR;
    if constexpr (std::same_as<T, PtrType>)
      return TYPE_PTR;
    if constexpr (std::same_as<T, MutPtrType>)
      return TYPE_MUT_PTR;
  }
}

#endif //!HG_COLTC_TYPE