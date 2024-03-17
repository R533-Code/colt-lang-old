/*****************************************************************//**
 * @file   colt_type.h
 * @brief  Contains the types as represented by the colt compiler.
 * 
 * @author RPC
 * @date   February 2024
 *********************************************************************/
#ifndef HG_COLTC_TYPE
#define HG_COLTC_TYPE

#include "util/types.h"
#include "meta/meta_type_list.h"
#include "colt_builtin_id.h"
#include "util/hash.h"
#include "colt_type_token.h"
#include "macro_helper.h"

DECLARE_ENUM_WITH_TYPE(u8, clt::lng, TypeID,
  TYPE_ERROR, TYPE_BUILTIN, TYPE_VOID,
  TYPE_PTR, TYPE_MUT_PTR, TYPE_OPTR, TYPE_MUT_OPTR
);

/// @brief Macro Type List (with same index as TypeID declaration!)
#define COLTC_TYPE_LIST ErrorType, BuiltinType, VoidType, \
   PtrType, MutPtrType, OpaquePtrType, MutOpaquePtrType

namespace clt::lng
{
  // Forward declarations
  FORWARD_DECLARE_TYPE_LIST(COLTC_TYPE_LIST);
  // TypeToTypeID
  CONVERT_TYPE_TO_ENUM(TypeID, COLTC_TYPE_LIST);

  /// @brief Base class of all types
  class TypeBase
  {
    /// @brief The ID of the type (used for down-casts)
    TypeID type_id;

  public:
    // No default constructor
    constexpr TypeBase() noexcept = delete;
    /// @brief Constructs a TypeBase
    /// @param id The Type ID
    constexpr TypeBase(TypeID id) noexcept
      : type_id(id) {}    
    MAKE_DEFAULT_COPY_AND_MOVE_FOR(TypeBase);

    /// @brief Returns the ID of the type (used for down-casts)
    /// @return The TypeID of the current type
    constexpr TypeID classof() const noexcept { return type_id; }
  };

  template<typename T>
  /// @brief A ColtType provides its ID and is equality comparable
  concept ColtType = std::equality_comparable<T> && std::convertible_to<T, TypeBase> && requires (T a)
  {
    { a.classof() } -> std::same_as<TypeID>;
    { a.getHash() } -> std::same_as<size_t>;
  };  

  // Create a type that is default constructible, movable and move assignable
#define CREATE_TYPE(name) class name final : public TypeBase \
                          { \
                          public: \
                            constexpr name() noexcept : TypeBase(TypeToTypeID<name>()) {} \
                            MAKE_DEFAULT_COPY_AND_MOVE_FOR(name) \
                            constexpr bool operator==(const name&) const { return true; } \
                            constexpr size_t getHash() const noexcept { return hash_value(static_cast<u8>(classof())); } \
                          }

  // Create the empty types.
  // These types have the default constructor,
  // and an equality comparison operator.

  /// @brief Represents an error type.
  /// This type is used by the compiler to avoid generating
  /// a lot of error when parsing an invalid type.
  CREATE_TYPE(ErrorType);
  /// @brief Represents the absence of a type.
  CREATE_TYPE(VoidType);
  /// @brief Represents a pointer to const void (opaque ptr)
  CREATE_TYPE(OpaquePtrType);
  /// @brief Represents a pointer to void (mut opaque ptr)
  CREATE_TYPE(MutOpaquePtrType);

#undef CREATE_TYPE

  /// @brief Represents a built-in type (integer, floating point...)
  class BuiltinType
    final : public TypeBase
  {
    /// @brief The built-in type ID
    BuiltinID type_id;

  public:
    /// @brief Constructs a built-in type
    /// @param id The built-in type ID
    constexpr BuiltinType(BuiltinID id) noexcept
      : TypeBase(TypeToTypeID<BuiltinType>()), type_id(id) {}
    // No default constructor
    BuiltinType() = delete;
    MAKE_DEFAULT_COPY_AND_MOVE_FOR(BuiltinType);

    /// @brief Check if two built-in types represent the same types
    /// @param b The other built-in type
    /// @return True if both built-in ID are the same
    constexpr bool operator==(const BuiltinType& b) const noexcept { return type_id == b.type_id; }

    /// @brief Returns the built-in type ID
    /// @return The built-in type ID
    constexpr BuiltinID typeID() const noexcept { return type_id; }

    /// @brief Hashes the current type
    /// @return The hash of the current type
    constexpr size_t getHash() const noexcept
    {
      size_t seed = 0;
      seed = hash_combine(seed, hash_value(static_cast<u8>(classof())));
      seed = hash_combine(seed, hash_value(static_cast<u8>(typeID())));
      return seed;
    }
  };

  /// @brief Represents a pointer to constant memory of a type
  class PtrType
    final : public TypeBase
  {
    /// @brief The type index pointed to
    TypeToken type_id;

  public:
    /// @brief Constructor
    /// @param id The type pointer to
    constexpr PtrType(TypeToken id) noexcept
      : TypeBase(TypeToTypeID<PtrType>()), type_id(id) {}
    // No default constructor
    PtrType() = delete;
    MAKE_DEFAULT_COPY_AND_MOVE_FOR(PtrType);

    /// @brief Check if two ptr types represent the same type
    /// @param b The other pointer type
    /// @return True if both types point to the same type
    constexpr bool operator==(const PtrType& b) const noexcept { return type_id == b.type_id; }

    /// @brief Returns the type pointed to
    /// @return The type pointed to
    constexpr TypeToken getPointingTo() const noexcept { return type_id; }
    
    /// @brief Hashes the current type
    /// @return The hash of the current type
    constexpr size_t getHash() const noexcept
    {
      size_t seed = 0;
      seed = hash_combine(seed, hash_value(static_cast<u8>(classof())));
      seed = hash_combine(seed, hash_value(getPointingTo().getID()));
      return seed;
    }
  };
  
  /// @brief Represents a pointer to mutable memory of a type
  class MutPtrType
    final : public TypeBase
  {
    /// @brief The type index pointed to
    TypeToken type_id;

  public:
    /// @brief Constructor
    /// @param id The type pointer to
    constexpr MutPtrType(TypeToken id) noexcept
      : TypeBase(TypeToTypeID<MutPtrType>()), type_id(id) {}
    // No default constructor
    MutPtrType() = delete;
    MAKE_DEFAULT_COPY_AND_MOVE_FOR(MutPtrType);

    /// @brief Check if two ptr types represent the same type
    /// @param b The other pointer type
    /// @return True if both types point to the same type
    constexpr bool operator==(const MutPtrType& b) const noexcept { return type_id == b.type_id; }

    /// @brief Returns the type pointed to
    /// @return The type pointed to
    constexpr TypeToken getPointingTo() const noexcept { return type_id; }

    /// @brief Hashes the current type
    /// @return The hash of the current type
    constexpr size_t getHash() const noexcept
    {
      size_t seed = 0;
      seed = hash_combine(seed, hash_value(static_cast<u8>(classof())));
      seed = hash_combine(seed, hash_value(getPointingTo().getID()));
      return seed;
    }
  };  

  /// @brief Type List of all Colt Types
  using ColtTypeList = meta::type_list<COLTC_TYPE_LIST>;

  static_assert(ColtTypeList::size == reflect<TypeID>::count(),
    "ColtTypeList and TypeID count must be equal!");

  /// @brief Macro helper to generate function table
#define COLTC_EXPAND_VARIANT_ARG(template_fn, value) , &template_fn<value>
  /// @brief Macro helper to generate function table
#define COLTC_TYPE_VARIANT_IMPL_GEN_TABLE(template_fn, first, ...) \
  std::array{ &template_fn<first> COLT_FOR_EACH_1ARG(COLTC_EXPAND_VARIANT_ARG, template_fn, __VA_ARGS__) }
  /// @brief Macro used inside of TypeVariant, see OperatorEqualTable
#define COLTC_TYPE_VARIANT_GEN_TABLE(template_fn, list) COLTC_TYPE_VARIANT_IMPL_GEN_TABLE(template_fn, list)

  /// @brief Macro used to generate getUnionMember body
#define COLTC_getUnionMember \
  if constexpr (std::same_as<T, ErrorType>) \
    return error_type; \
  if constexpr (std::same_as<T, VoidType>) \
    return void_type; \
  if constexpr (std::same_as<T, OpaquePtrType>) \
    return opaque_type; \
  if constexpr (std::same_as<T, MutOpaquePtrType>) \
    return mut_opaque_type; \
  if constexpr (std::same_as<T, BuiltinType>) \
    return builtin_type; \
  if constexpr (std::same_as<T, PtrType>) \
    return ptr_type; \
  if constexpr (std::same_as<T, MutPtrType>) \
    return mut_ptr_type

  /// @brief Represents a type.
  /// Rather than using an inheritance, we make use of a variant.
  /// This variant is responsible of runtime polymorphism through
  /// function dispatch.
  class TypeVariant
  {
    template<typename T, typename... Args>
    /// @brief Helper to construct the union
    /// @param placement The pointer to the object to construct
    /// @param args... The arguments to forward to the constructor
    /// @return Empty monostate
    static constexpr std::monostate construct(T* placement, Args&&... args)
    {
      // Use construct_at rather than placement new for constexpr
      std::construct_at(placement, std::forward<Args>(args)...);
      return {};
    }

    /// @brief The possible types contained in the variant
    union
    {
      // Helper used in constructor
      std::monostate   _mono_state_;
      /// @brief Access through ErrorType
      ErrorType        error_type;
      /// @brief Access through VoidType
      VoidType         void_type;
      /// @brief Access through OpaquePtrType
      OpaquePtrType    opaque_type;
      /// @brief Access through MutOpaquePtrType
      MutOpaquePtrType mut_opaque_type;
      /// @brief Access through BuiltinType
      BuiltinType      builtin_type;
      /// @brief Access through PtrType
      PtrType          ptr_type;
      /// @brief Access through MutPtrType
      MutPtrType       mut_ptr_type;      
    };

    template<typename T>
    /// @brief Returns a reference to the union member of type 'T'
    /// @return Reference to the union member of type 'T'
    constexpr const auto& getUnionMember() const noexcept { COLTC_getUnionMember; }
    
    template<typename T>
    /// @brief Returns a reference to the union member of type 'T'
    /// @return Reference to the union member of type 'T'
    constexpr auto& getUnionMember() noexcept { COLTC_getUnionMember; }

    // Generates a dispatch table for using table_operator_equal.
    // The generated dispatch table can be indexed by getTypeID().
    
    template<typename T>
    /// @brief Compares two variants if using their operator=
    /// @param a The first variant
    /// @param b The second variant
    /// @return True if a.getUnionMember<T>() == b.getUnionMember<T>() evaluates to true
    static constexpr bool table_equal(const TypeVariant& a, const TypeVariant& b) noexcept
    {
      return a.getUnionMember<T>() == b.getUnionMember<T>();
    }

    template<typename T>
    /// @brief Hashes a variant if using their getHash() method
    /// @param a The variant to hash
    /// @return a.getUnionMember<T>().getHash()
    static constexpr bool table_hash(const TypeVariant& a) noexcept
    {
      return a.getUnionMember<T>().getHash();
    }
    
    static constexpr auto EqualTable = COLTC_TYPE_VARIANT_GEN_TABLE(table_equal, COLTC_TYPE_LIST);
    static constexpr auto HashTable  = COLTC_TYPE_VARIANT_GEN_TABLE(table_hash, COLTC_TYPE_LIST);

  public:
    template<ColtType Type, typename... Args>
    /// @brief Constructor
    /// @param args... The arguments to forward to the constructor
    constexpr TypeVariant(std::type_identity<Type>, Args&&... args)
      noexcept(std::is_nothrow_constructible_v<Type, Args...>)
      : _mono_state_(construct<Type>(&getUnionMember<Type>(), std::forward<Args>(args)...))
    {
      // The ugly code above is used to keep the constructor constexpr
    }

    constexpr TypeVariant(TypeVariant&&) noexcept = default;
    constexpr TypeVariant(const TypeVariant&) noexcept = default;
    constexpr TypeVariant& operator=(TypeVariant&&) noexcept = default;
    constexpr TypeVariant& operator=(const TypeVariant&) noexcept = default;

    /// @brief Returns the type ID of the current type
    /// @return The ID of the current type
    constexpr TypeID getTypeID() const noexcept
    {
      // We cannot read the value through any of the members
      // because of UB.
      // The memcpy is optimized away by the compiler.
      TypeID id;
      if (std::is_constant_evaluated()) // bit_cast is constexpr...
        id = std::bit_cast<TypeID>(error_type);
      else
        std::memcpy(&id, &error_type, sizeof(TypeID));
      return id;
    }

    /// @brief Check if the type is a pointer to mutable
    /// @return True if TYPE_MUT_OPTR or TYPE_MUT_PTR
    constexpr bool isMutPtr() const noexcept
    {
      return getTypeID() == TypeID::TYPE_MUT_OPTR || getTypeID() == TypeID::TYPE_MUT_PTR;
    }

    /// @brief Check if the type is a pointer to mutable
    /// @return True if TYPE_MUT_OPTR or TYPE_MUT_PTR
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
        && check(builtin_type.typeID());
    }

    /// @brief Check if the current type is void
    constexpr bool isVoid()    const noexcept { return getTypeID() == TypeID::TYPE_VOID; }
    /// @brief Check if the current type is an error
    constexpr bool isError()   const noexcept { return getTypeID() == TypeID::TYPE_ERROR; }
    /// @brief Check if the current type is a built-in type
    constexpr bool isBuiltin() const noexcept { return getTypeID() == TypeID::TYPE_BUILTIN; }

    /// @brief Check if the current type is equal to another type
    /// @param type The other type to compare to
    /// @return True of the types are the same
    constexpr bool operator==(const TypeVariant& type) const
    {
      if (this->getTypeID() != type.getTypeID())
        return false;

      return EqualTable[static_cast<u8>(this->getTypeID())](*this, type);
    }
    
    /// @brief Check if the current type is same as another.
    /// The difference between isSameAs and operator== is that
    /// if any of the types compared are errors isSameAs returns true.
    /// @param type The other type to compare to
    /// @return True if any of the types are errors or if the types are the same
    constexpr bool isSameAs(const TypeVariant& type) const noexcept
    {
      if (this->isError() || type.isError())
        return true;
      return *this == type;
    }

    template<ColtType T>
    /// @brief Casts the current type to 'T'
    /// @tparam T The type to cast to
    /// @return nullptr if the variant does not contain 'T', or valid pointer to 'T'
    constexpr T* getType() noexcept
    {
      if (getTypeID() != TypeToTypeID<T>())
        return nullptr;
      return &getUnionMember<T>();
    }

    template<ColtType T>
    /// @brief Casts the current type to 'T'
    /// @tparam T The type to cast to
    /// @return nullptr if the variant does not contain 'T', or valid pointer to 'T'
    constexpr const T* getType() const noexcept
    {
      if (getTypeID() != TypeToTypeID<T>())
        return nullptr;
      return &getUnionMember<T>();
    }

    /// @brief Hashes the current type.
    /// The hash of different type might be equal!
    /// @return The hash of the current type
    constexpr size_t getHash() const noexcept
    {
      return HashTable[static_cast<u8>(this->getTypeID())](*this);
    }
  };

  template<ColtType type, typename... Args>
  /// @brief Constructs a TypeVariant containing a type
  /// @param args... The argument to forward to the constructor
  /// @return TypeVariant containing the constructed type
  constexpr TypeVariant make_coltc_type(Args&&... args) noexcept(std::is_nothrow_constructible_v<type, Args...>)
  {
    return TypeVariant(std::type_identity<type>{}, std::forward<Args>(args)...);
  }
  
  /// @brief Table of all built-in types
  static constexpr std::array ColtBuiltinTypeTable =
  {
    make_coltc_type<BuiltinType>(BuiltinID::BOOL),
    make_coltc_type<BuiltinType>(BuiltinID::CHAR),
    make_coltc_type<BuiltinType>(BuiltinID::U8),
    make_coltc_type<BuiltinType>(BuiltinID::U16),
    make_coltc_type<BuiltinType>(BuiltinID::U32),
    make_coltc_type<BuiltinType>(BuiltinID::U64),
    make_coltc_type<BuiltinType>(BuiltinID::I8),
    make_coltc_type<BuiltinType>(BuiltinID::I16),
    make_coltc_type<BuiltinType>(BuiltinID::I32),
    make_coltc_type<BuiltinType>(BuiltinID::I64),
    make_coltc_type<BuiltinType>(BuiltinID::F32),
    make_coltc_type<BuiltinType>(BuiltinID::F64),
    make_coltc_type<BuiltinType>(BuiltinID::BYTE),
    make_coltc_type<BuiltinType>(BuiltinID::WORD),
    make_coltc_type<BuiltinType>(BuiltinID::DWORD),
    make_coltc_type<BuiltinType>(BuiltinID::QWORD)
  };
}

namespace clt
{
  template<>
  struct hash<lng::TypeVariant>
  {
    constexpr size_t operator()(const lng::TypeVariant& var) noexcept
    {
      return var.getHash();
    }
  };
}

#undef COLTC_getUnionMember
#undef COLTC_TYPE_VARIANT_GEN_TABLE
#undef COLTC_TYPE_VARIANT_IMPL_GEN_TABLE
#undef COLTC_EXPAND_VARIANT_ARG

#endif //!HG_COLTC_TYPE