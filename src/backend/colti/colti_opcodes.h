#ifndef HG_COLTI_OPCODES
#define HG_COLTI_OPCODES

#include "common/types.h"
#include "run/qword_op.h"

DECLARE_ENUM_WITH_TYPE(
    u8, clt::run, OpCodeFamily,
    // [3b:FAMILY = BINARY] [4b: TypeOp] [0]
    // [0000] [4b:OpCodeBinary]
    ARITHMETIC, BITWISE,
    // [3b: FAMILY = IMMEDIATE] [3b: Number of Bytes Following] [00]
    // [8b]{Number of Bytes Following}
    IMMEDIATE,
    // [3b: FAMILY = CONVERSION] [0 0000]
    // [4b: From TypeOp] [4b: To TypeOp]
    CONVERSION, BRANCH, FII);

/// @brief The arithmetic operations.
DECLARE_ENUM_WITH_TYPE(
    u8, clt::run, OpCodeArithmetic, _add, _sub, _mul, _div, _mod, _neg, _eq, _neq,
    _le, _ge, _leq, _geq);

DECLARE_ENUM_WITH_TYPE(
    u8, clt::run, OpCodeBitwise, _and, _or, _xor, _lsr, _lsl, _asr, _not);

DECLARE_ENUM_WITH_TYPE(
    u8, clt::run, OpCodeBranch, _bt, _bti, _bf, _bfi, _b, _bi, _call, _calli, _ret);

namespace clt::run
{
}

#endif // !HG_COLTI_OPCODES