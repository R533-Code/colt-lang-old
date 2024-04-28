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
#include "structs/vector.h"
#include "util/hash.h"
#include "colt_type_token.h"
#include "colt_support_op.h"
#include "macro_helper.h"

DECLARE_ENUM_WITH_TYPE(u8, clt::lng, TypeID,
  TYPE_ERROR, TYPE_BUILTIN, TYPE_VOID,
  TYPE_PTR, TYPE_MUT_PTR, TYPE_OPTR, TYPE_MUT_OPTR, TYPE_FN
);

/// @brief Macro Type List (with same index as TypeID declaration!)
#define COLTC_TYPE_LIST ErrorType, BuiltinType, VoidType, \
   PtrType, MutPtrType, OpaquePtrType, MutOpaquePtrType, FnType

DECLARE_ENUM_WITH_TYPE(u8, clt::lng, ArgSpecifier,
  ARG_IN, ARG_OUT, ARG_INOUT, ARG_MOVE);

namespace clt::lng
{
  // Forward declarations
  FORWARD_DECLARE_TYPE_LIST(COLTC_TYPE_LIST);
  // TypeToTypeID
  CONVERT_TYPES_TO_ENUM(TypeID, COLTC_TYPE_LIST);

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

  class TypeVariant;
  
  template<typename T>
  /// @brief A ColtType provides its ID and is equality comparable
  concept ColtType = std::equality_comparable<T> && std::convertible_to<T, TypeBase> &&
    requires (const T a, UnaryOp unary, BinaryOp binary, const TypeVariant& var)
  {
    { a.classof() } -> std::same_as<TypeID>;
    { a.getHash() } -> std::same_as<size_t>;
    { a.supports(unary) } -> std::same_as<UnarySupport>;
    { a.supports(binary, var) } -> std::same_as<BinarySupport>;
  };

  // Create a type that is default constructible, movable and move assignable
#define CREATE_TYPE(name, unary_support) class name final : public TypeBase \
                          { \
                          public: \
                            constexpr name() noexcept : TypeBase(TypeToTypeID<name>()) {} \
                            MAKE_DEFAULT_COPY_AND_MOVE_FOR(name) \
                            constexpr bool operator==(const name&) const { return true; } \
                            constexpr size_t getHash() const noexcept { return hash_value(static_cast<u8>(classof())); } \
                            constexpr UnarySupport supports(UnaryOp op) const noexcept { return unary_support(op); } \
                            BinarySupport supports(BinaryOp op, const TypeVariant& var) const noexcept; \
                          }

  // Create the empty types.
  // These types have the default constructor,
  // and an equality comparison operator.

  /// @brief Represents an error type.
  /// This type is used by the compiler to avoid generating
  /// a lot of error when parsing an invalid type.
  CREATE_TYPE(ErrorType, ErrorSupport);
  /// @brief Represents the absence of a type.
  CREATE_TYPE(VoidType, NoSupport);
  /// @brief Represents a pointer to const void (opaque ptr)
  CREATE_TYPE(OpaquePtrType, PtrSupport);
  /// @brief Represents a pointer to void (mut opaque ptr)
  CREATE_TYPE(MutOpaquePtrType, PtrSupport);

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

    /// @brief Check if the current type supports 'op'
    /// @param op The operator whose support to check
    /// @return INVALID or BUILTIN
    constexpr UnarySupport supports(UnaryOp op) const noexcept { return BuiltinSupport(typeID(), op); }
    BinarySupport supports(BinaryOp op, const TypeVariant& var) const noexcept;
  };

  class PointerType
    : public TypeBase
  {
    /// @brief The type index pointed to
    TypeToken type_id;

  public:
    /// @brief Constructor
    /// @param type The type of the pointer
    /// @param id The type pointer to
    constexpr PointerType(TypeID type, TypeToken id) noexcept
      : TypeBase(type), type_id(id)
    {
      assert_true("Expected pointer type!", type == TypeID::TYPE_MUT_PTR || type == TypeID::TYPE_PTR);
    }

    /// @brief Check if the pointer type is mutable or not
    /// @return True if classof() == TYPE_MUT_PTR
    constexpr bool isMut() const noexcept { return classof() == TypeID::TYPE_MUT_PTR; }

    /// @brief Returns the type pointed to
    /// @return The type pointed to
    constexpr TypeToken getPointingTo() const noexcept { return type_id; }

    constexpr UnarySupport supports(UnaryOp op) const noexcept { return PtrSupport(op); }
    BinarySupport supports(BinaryOp op, const TypeVariant& var) const noexcept;
  };

  /// @brief Represents a pointer to constant memory of a type
  class PtrType
    final : public PointerType
  {
  public:    
    constexpr PtrType(TypeToken id) noexcept
      : PointerType(TypeToTypeID<PtrType>(), id) {}

    // No default constructor
    PtrType() = delete;
    MAKE_DEFAULT_COPY_AND_MOVE_FOR(PtrType);

    /// @brief Check if two ptr types represent the same type
    /// @param b The other pointer type
    /// @return True if both types point to the same type
    constexpr bool operator==(const PtrType& b) const noexcept { return getPointingTo() == b.getPointingTo(); }

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
    final : public PointerType
  {
  public:
    /// @brief Constructor
    /// @param id The type pointer to
    constexpr MutPtrType(TypeToken id) noexcept
      : PointerType(TypeToTypeID<MutPtrType>(), id) {}
    // No default constructor
    MutPtrType() = delete;
    MAKE_DEFAULT_COPY_AND_MOVE_FOR(MutPtrType);

    /// @brief Check if two ptr types represent the same type
    /// @param b The other pointer type
    /// @return True if both types point to the same type
    constexpr bool operator==(const MutPtrType& b) const noexcept { return getPointingTo() == b.getPointingTo(); }

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

  /// @brief Represents a function type argument
  struct FnTypeArgument
  {
    /// @brief The type of the function
    TypeToken type;
    /// @brief The specifier applied to the type
    ArgSpecifier specifier;

    /// @brief Compares for equality
    /// @return True if both objects are equal
    constexpr bool operator==(const FnTypeArgument&) const noexcept = default;
  };

  /// @brief Represents the payload of a function type.
  /// To keep the size of the TypeVariant small, we make
  /// use of an array of FnTypePayload (stored in TypeBuffer)
  /// and only an index into that array is stored in FnType.
  struct FnTypePayload
  {
    /// @brief True if the function uses C variadic
    u8 is_variadic : 1;
    /// @brief The return type of the function (which can be void)
    TypeToken return_type;
    //TODO: Replace by SmallVector
    /// @brief The function arguments
    Vector<FnTypeArgument> arguments_type;

    /// @brief Compares for equality
    /// @return True if both objects are equal
    constexpr bool operator==(const FnTypePayload& b) const noexcept
    {
      return is_variadic == b.is_variadic || return_type == b.return_type
        || arguments_type == b.arguments_type.to_view();
    }
  };

  /// @brief Represents a function type
  class FnType
    final : public TypeBase
  {
    /// @brief Index into the set of FnTypePayload
    u32 payload_index;
  
  public:
    /// @brief Constructor
    /// @param payload_index The index into the set of FnTypePayload
    constexpr FnType(u32 payload_index) noexcept
      : TypeBase(TypeToTypeID<FnType>()), payload_index(payload_index) {}
    // No default constructor
    FnType() = delete;
    MAKE_DEFAULT_COPY_AND_MOVE_FOR(FnType);

    /// @brief Check if two ptr types represent the same type
    /// @param b The other pointer type
    /// @return True if both types point to the same type
    constexpr bool operator==(const FnType& b) const noexcept { return payload_index == b.payload_index; }

    /// @brief Hashes the current type
    /// @return The hash of the current type
    constexpr size_t getHash() const noexcept
    {
      size_t seed = 0;
      seed = hash_combine(seed, hash_value(static_cast<u8>(classof())));
      seed = hash_combine(seed, hash_value(payload_index));
      return seed;
    }

    /// @brief Check if the current type supports 'op'
    /// @param op The operator whose support to check
    /// @return INVALID
    constexpr UnarySupport supports(UnaryOp op) const noexcept { return NoSupport(op); }
    BinarySupport supports(BinaryOp op, const TypeVariant& var) const noexcept;
  };

  template<typename T>
  struct type_group_requirements
  {
    template<typename Ty>
    struct base_of
    {
      static constexpr bool value = std::is_base_of_v<T, Ty>;
    };

    using type = meta::type_list<COLTC_TYPE_LIST>::remove_if_not<base_of>;
  };

  template<typename T>
  using type_group_requirements_t = typename type_group_requirements<T>::type;

  /// @brief Type List of all Colt Types
  using ColtTypeList = meta::type_list<COLTC_TYPE_LIST>;

  static_assert(ColtTypeList::size == reflect<TypeID>::count(),
    "ColtTypeList and TypeID count must be equal!");

  /// @brief Macro helper to generate function table
#define COLTC_EXPAND_VARIANT_ARG(template_fn, value) , &template_fn<clt::lng:: value>
  /// @brief Macro helper to generate function table
#define COLTC_TYPE_VARIANT_IMPL_GEN_TABLE(template_fn, first, ...) \
  std::array{ &template_fn<clt::lng:: first> COLT_FOR_EACH_1ARG(COLTC_EXPAND_VARIANT_ARG, template_fn, __VA_ARGS__) }
  /// @brief Macro used inside of TypeVariant, see OperatorEqualTable
#define COLTC_TYPE_VARIANT_GEN_TABLE(template_fn, list) COLTC_TYPE_VARIANT_IMPL_GEN_TABLE(template_fn, list)

  /// @brief Represents a type.
  /// Rather than using an inheritance, we make use of a variant.
  /// This variant is responsible of runtime polymorphism through
  /// function dispatch.
  class TypeVariant
  {
    MAKE_UNION_AND_GET_MEMBER(COLTC_TYPE_LIST);

    template<typename... Args>
    constexpr bool is_classof_any_of(meta::type_list<Args...>) const noexcept
    {
      return (... || (classof() == TypeToTypeID<Args>()));
    }

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
    
    template<typename T>
    /// @brief Check for 'op' support
    /// @param a The variant to hash
    /// @param op The unary operation to check for
    /// @return a.getUnionMember<T>().supports(op)
    static constexpr UnarySupport table_supports_u(const TypeVariant& a, UnaryOp op) noexcept
    {
      return a.getUnionMember<T>().supports(op);
    }
    
    template<typename T>
    /// @brief Check for 'op' support
    /// @param a The variant to hash
    /// @param op The unary operation to check for
    /// @param var The right hand side
    /// @return a.getUnionMember<T>().supports(op)
    static constexpr BinarySupport table_supports_b(const TypeVariant& a, BinaryOp op, const TypeVariant& var) noexcept
    {
      return a.getUnionMember<T>().supports(op, var);
    }
    
    static constexpr auto EqualTable      = COLTC_TYPE_VARIANT_GEN_TABLE(table_equal, COLTC_TYPE_LIST);
    static constexpr auto HashTable       = COLTC_TYPE_VARIANT_GEN_TABLE(table_hash, COLTC_TYPE_LIST);
    static constexpr auto USupportsTable  = COLTC_TYPE_VARIANT_GEN_TABLE(table_supports_u, COLTC_TYPE_LIST);
    static constexpr auto BSupportsTable  = COLTC_TYPE_VARIANT_GEN_TABLE(table_supports_b, COLTC_TYPE_LIST);

  public:
    template<ColtType Type, typename... Args>
    /// @brief Constructor
    /// @param args... The arguments to forward to the constructor
    constexpr TypeVariant(std::type_identity<Type>, Args&&... args)
      noexcept(std::is_nothrow_constructible_v<Type, Args...>)
    {
      construct<Type>(&getUnionMember<Type>(), std::forward<Args>(args)...);
    }

    constexpr TypeVariant(TypeVariant&&) noexcept = default;
    constexpr TypeVariant(const TypeVariant&) noexcept = default;
    constexpr TypeVariant& operator=(TypeVariant&&) noexcept = default;
    constexpr TypeVariant& operator=(const TypeVariant&) noexcept = default;

    /// @brief Returns the type ID of the current type
    /// @return The ID of the current type
    constexpr TypeID getTypeID() const noexcept
    {
      // This is likely UB...
      TypeID id;
      if (std::is_constant_evaluated()) // bit_cast is constexpr...
        id = std::bit_cast<TypeID>(_ErrorType);
      else
        id = ((const TypeBase*)&_buffer)->classof();
      return id;
    }

    /// @brief Returns the type ID of the current type
    /// @return The ID of the current type
    constexpr TypeID classof() const noexcept { return getTypeID(); }

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

    /// @brief Check if the type is a pointer (optionally to mutable)
    /// @return True if isPtr or isMutPtr
    constexpr bool isAnyPtr() const noexcept
    {
      return isMutPtr() || isPtr();
    }

    /// @brief Check if the type is an opaque (possibly mutable) pointer 
    /// @return True if mut? opaque pointer
    constexpr bool isAnyOpaquePtr() const noexcept
    {
      return getTypeID() == TypeID::TYPE_OPTR || getTypeID() == TypeID::TYPE_MUT_OPTR;
    }
    
    /// @brief Check if the current type is built-in and 'check' its ID.
    /// The check is performed with the built-in ID only if the current type
    /// is built-in.
    /// @param check The function to check for if built-in
    /// @return True only if built-in and 'check' returns true
    constexpr bool isBuiltinAnd(BuilinTypeCheck_t check) const noexcept
    {
      return getTypeID() == TypeID::TYPE_BUILTIN
        && check(_BuiltinType.typeID());
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
    constexpr T* as() noexcept
    {
      if (getTypeID() != TypeToTypeID<T>())
        return nullptr;
      return &getUnionMember<T>();
    }

    template<ColtType T>
    /// @brief Casts the current type to 'T'
    /// @tparam T The type to cast to
    /// @return nullptr if the variant does not contain 'T', or valid pointer to 'T'
    constexpr const T* as() const noexcept
    {
      if (getTypeID() != TypeToTypeID<T>())
        return nullptr;
      return &getUnionMember<T>();
    }

    template<typename T>
    /// @brief Downcasts the variant to 'T'
    /// @return nullptr if type does not match else pointer to the type
    constexpr const T* as() const noexcept
    {
      static_assert(type_group_requirements_t<T>::size != 0, "Group must be inherited from!");
      if (!is_classof_any_of(type_group_requirements_t<T>{}))
        return nullptr;
      return (const T*)&_buffer;
    }

    /// @brief Hashes the current type.
    /// The hash of different type might be equal!
    /// @return The hash of the current type
    constexpr size_t getHash() const noexcept
    {
      return HashTable[static_cast<u8>(this->getTypeID())](*this);
    }

    /// @brief Check if the current type supports 'op'
    /// @param op The operator to check for
    /// @return UnarySupport
    constexpr UnarySupport supports(UnaryOp op) const noexcept
    {
      return USupportsTable[static_cast<u8>(this->getTypeID())](*this, op);
    }

    /// @brief Check if the current type supports 'op'
    /// @param op The operator to check for
    /// @param var The right hand side
    /// @return BinaryOp
    constexpr BinarySupport supports(BinaryOp op, const TypeVariant& var) const noexcept
    {
      return BSupportsTable[static_cast<u8>(this->getTypeID())](*this, op, var);
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

  template<>
  struct hash<lng::FnTypeArgument>
  {
    constexpr size_t operator()(const lng::FnTypeArgument& var) noexcept
    {
      size_t seed = 0;
      seed = hash_combine(seed, hash_value((u8)var.specifier));
      seed = hash_combine(seed, hash_value(var.type.getID()));
      return seed;
    }
  };
  
  template<>
  struct hash<lng::FnTypePayload>
  {
    constexpr size_t operator()(const lng::FnTypePayload& var) noexcept
    {
      size_t seed = 0;
      seed = hash_combine(seed, hash_value(var.is_variadic));
      seed = hash_combine(seed, hash_value(var.return_type.getID()));
      seed = hash_combine(seed, hash_value(var.arguments_type.to_view()));
      return seed;
    }
  };
}

#undef COLTC_TYPE_VARIANT_GEN_TABLE
#undef COLTC_TYPE_VARIANT_IMPL_GEN_TABLE
#undef COLTC_EXPAND_VARIANT_ARG

#endif //!HG_COLTC_TYPE