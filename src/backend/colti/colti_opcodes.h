#ifndef HG_COLTI_OPCODES
#define HG_COLTI_OPCODES

#include "common/types.h"
#include "run/qword_op.h"

namespace clt::run
{
  /// @brief Represents the internal encoding of the instruction.
  /// All instructions are encoded as 8 bytes.
  enum class InstEncoding
  {
    /// @brief Represents a binary operation of the form
    /// DEST = A Operation B.
    /// This represents instruction similar to 'add' or 'sub'.
    /// [OpCode: 0000][Operation: 4b] [DEST: 8b] [A: 8b] [B: 8b]
    /// [TypeOp: 4b][0000] [0: 8b]*3
    BINARY_TYPE,
    /// @brief Represents a binary operation of the form
    /// DEST = (A Operation B) & ((2 << n) - 1).
    /// This represents instruction similar to 'and' or 'or'
    /// [OpCode: 0001][Operation: 4b] [DEST: 8b] [A: 8b] [B: 8b]
    /// [n: 6b][00] [0: 8b]*3
    BINARY_BITS,
    /// @brief Represents a branch operation.
    /// This represents instruction similar to 'call' and 'b'.
    /// [OpCode: 0010][Operation: 4b] [Signed offset: 56b]
    BRANCH,
    /// @brief Represents a signed immediate load instruction.
    /// [OpCode: 0011][Signed Immediate: 60b]
    SIGNED_IMM,
    /// @brief Represents an unsigned immediate load instruction.
    /// [OpCode: 0100][Unsigned Immediate: 60b]
    UNSIGNED_IMM,
  };

  /// @brief Represents a binary typed instruction
  class BinaryTypeInst
  {
    enum class Field
    {
      /// @brief [0000]
      OpCode,
      /// @brief The operation to execute
      Operation,
      /// @brief The destination register
      Dest,
      /// @brief The first operand
      A,
      /// @brief The second operand
      B,
      /// @brief The type
      Type,
      /// @brief Unused for now
      Padding
    };

    using _type = Bitfields<
        u64, Bitfield<Field::OpCode, 4>, Bitfield<Field::Operation, 4>,
        Bitfield<Field::Dest, 8>, Bitfield<Field::A, 8>,
        Bitfield<Field::B, 8>, Bitfield<Field::Type, 4>,
        Bitfield<Field::Padding, 28>>;

    _type storage{};

  public:
    /// @brief Represents the possible operations
    enum class Op : u8
    {
      add,
      sub,
      mul,
      div,
      mod,
      eq,
      neq,
      le,
      ge,
      leq,
      geq,
    };

    /// @brief Constructor
    /// @param operation The operation of the instruction
    /// @param dest The destination register
    /// @param op1 The first operand register
    /// @param op2 The second operand register
    /// @param type The type of the operands
    constexpr BinaryTypeInst(
        Op operation, u8 dest, u8 op1, u8 op2, TypeOp type) noexcept
    {
      using enum BinaryTypeInst::Field;

      storage.set<OpCode>((u64)InstEncoding::BINARY_TYPE);
      storage.set<Operation>((u64)operation);
      storage.set<Dest>((u64)dest);
      storage.set<A>((u64)op1);
      storage.set<B>((u64)op2);
      storage.set<Type>((u64)type);
    }

    /// @brief Returns the operation of the instruction
    /// @return The operation
    constexpr Op op() const noexcept { return (Op)storage.get<Field::Operation>(); }
    /// @brief Returns the destination register index
    /// @return The destination register
    constexpr u8 dest() const noexcept { return (u8)storage.get<Field::Dest>(); }
    /// @brief Returns the first operand register index
    /// @return The first operand register
    constexpr u8 op1() const noexcept { return (u8)storage.get<Field::A>(); }
    /// @brief Returns the second operand register index
    /// @return The second operand register
    constexpr u8 op2() const noexcept { return (u8)storage.get<Field::B>(); }
    /// @brief Returns the type on which to apply the operation
    /// @return The type on which to apply the operation
    constexpr TypeOp type() const noexcept
    {
      return (TypeOp)storage.get<Field::Type>();
    }
  };

  /// @brief Represents a binary bits instruction
  class BinaryBitsInst
  {
    enum class Field
    {
      /// @brief [0001]
      OpCode,
      /// @brief The operation to execute
      Operation,
      /// @brief The destination register
      Dest,
      /// @brief The first operand
      A,
      /// @brief The second operand
      B,
      /// @brief The number of bits to keep
      N,
      /// @brief Unused for now
      Padding
    };

    using _type = Bitfields<
        u64, Bitfield<Field::OpCode, 4>, Bitfield<Field::Operation, 4>,
        Bitfield<Field::Dest, 8>, Bitfield<Field::A, 8>,
        Bitfield<Field::B, 8>, Bitfield<Field::N, 6>,
        Bitfield<Field::Padding, 26>>;

    _type storage{};

  public:
    /// @brief Represents the possible operations
    enum class Op : u8
    {
      bit_and,
      bit_or,
      bit_xor,
      bit_lsr,
      bit_lsl,
      bit_asr,
    };

    /// @brief Constructor
    /// @param operation The operation to perform
    /// @param dest The destination register
    /// @param op1 The first operand register
    /// @param op2 The second operand
    /// @param n The number of bits to keep
    constexpr BinaryBitsInst(Op operation, u8 dest, u8 op1, u8 op2, u8 n) noexcept
    {
      using enum BinaryBitsInst::Field;

      assert_true("n must be between < 64", n < 64);
      storage.set<OpCode>(    (u64)InstEncoding::BINARY_BITS);
      storage.set<Operation>( (u64)operation);
      storage.set<Dest>(      (u64)dest);
      storage.set<A>(         (u64)op1);
      storage.set<B>(         (u64)op2);
      storage.set<N>(         (u64)n);
    }

    /// @brief Returns the operation of the instruction
    /// @return The operation
    constexpr Op op() const noexcept { return (Op)storage.get<Field::OpCode>(); }
    /// @brief Returns the destination register index
    /// @return The destination register
    constexpr u8 dest() const noexcept { return (u8)storage.get<Field::Dest>(); }
    /// @brief Returns the first operand register index
    /// @return The first operand register
    constexpr u8 op1() const noexcept { return (u8)storage.get<Field::A>(); }
    /// @brief Returns the second operand register index
    /// @return The second operand register
    constexpr u8 op2() const noexcept { return (u8)storage.get<Field::B>(); }
    /// @brief Returns the number of bits to keep after the operation
    /// @return The number of bits to keep after the operation
    constexpr u8 n() const noexcept { return (u8)storage.get<Field::N>(); }
  };

  /// @brief Represents a branch instruction
  class BranchInst
  {
    enum class Field
    {
      /// @brief [0010]
      OpCode,
      /// @brief The operation to execute
      Operation,
      /// @brief The offset to add to the PC
      Offset,
    };

    using _type = Bitfields<
        u64, Bitfield<Field::OpCode, 4>, Bitfield<Field::Operation, 4>,
        Bitfield<Field::Offset, 56>>;

    _type storage{};

  public:
    /// @brief Represents the possible operations
    enum class Op : u8
    {
      /// @brief Unconditional branch
      b,
      /// @brief Branch if true (non-zero)
      bt,
      /// @brief Branch if false (all-zero)
      bf,
      /// @brief Call function
      call,
    };

    /// @brief Constructor
    /// @param operation The operation to perform
    /// @param offset The signed offset (only 56 bits are used)
    constexpr BranchInst(Op operation, i64 offset) noexcept
    {
      using enum BranchInst::Field;

      assert_true(
          "Offset too great!", offset >= -36'028'797'018'963'968,
          offset < 36'028'797'018'963'968);
      storage.set<OpCode>(    (u64)InstEncoding::BRANCH);
      storage.set<Operation>( (u64)operation);
      storage.set<Offset>(    htol((u64)offset));
    }

    /// @brief Returns the operation of the instruction
    /// @return The operation
    constexpr Op op() const noexcept { return (Op)storage.get<Field::OpCode>(); }
    /// @brief Returns the signed offset to add to the program counter
    /// @return The signed offset
    constexpr i64 offset() const noexcept
    {
      return sign_extend(ltoh(storage.get<Field::Offset>()), 56);
    }
  };

#define COLT_INST_TYPE_LIST BinaryTypeInst, BinaryBitsInst, BranchInst

  class Inst
  {
    MAKE_UNION_AND_GET_MEMBER(COLT_INST_TYPE_LIST);

  public:
    template<typename Type, typename... Args>
      requires meta::is_any_of<Type, COLT_INST_TYPE_LIST>
    /// @brief Constructor
    /// @param args... The arguments to forward to the constructor
    constexpr Inst(std::type_identity<Type>, Args&&... args) noexcept(
        std::is_nothrow_constructible_v<Type, Args...>)
    {
      construct<Type>(&getUnionMember<Type>(), std::forward<Args>(args)...);
    }

    template<typename Type, typename... Args>
      requires meta::is_any_of<Type, COLT_INST_TYPE_LIST>
    static constexpr Inst make(Args&&... args) noexcept
    {
      return Inst(std::type_identity<Type>{}, std::forward<Args>(args)...);
    }

    constexpr bool operator==(const Inst& inst) const noexcept {}
  };

  static constexpr meta::ConstexprMap inst = std::array{std::pair{
      StringView{"add"},
      Inst::make<BinaryTypeInst>(BinaryTypeInst::Op::add, 0, 0, 0, TypeOp::i8_t)}};

  constexpr Option<Inst> find_instruction(StringView name)
  {    
    return inst.find(name);
  }
} // namespace clt::run

#endif // !HG_COLTI_OPCODES