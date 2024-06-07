#include "colt_type_buffer.h"

namespace clt::lng
{
  StringView getBuiltinName(BuiltinID ID) noexcept
  {
    using enum clt::lng::BuiltinID;
    
    switch_no_default (ID)
    {
    case BOOL:
      return "bool";
    case CHAR:
      return "char";
    case U8:
      return "u8";
    case U16:
      return "u16";
    case U32:
      return "u32";
    case U64:
      return "u64";
    case I8:
      return "i8";
    case I16:
      return "i16";
    case I32:
      return "i32";
    case I64:
      return "i64";
    case F32:
      return "f32";
    case F64:
      return "f64";
    case BYTE:
      return "BYTE";
    case WORD:
      return "WORD";
    case DWORD:
      return "DWORD";
    case QWORD:
      return "QWORD";
    }
  }

  StringView TypeBuffer::getTypeName(const TypeVariant& var) const noexcept
  {
    using enum TypeID;
    switch_no_default (var.classof())
    {
    case TYPE_ERROR:
      return "<ERROR>";
    case TYPE_BUILTIN:
      return getBuiltinName(var.as<BuiltinType>()->typeID());
    case TYPE_VOID:
      return "void";
    case TYPE_PTR:
    {
      type_names.push_back({});
      fmt::format_to(std::back_inserter(type_names.back()), "ptr.{}", getTypeName(var.as<PtrType>()->getPointingTo()));
      return type_names.back();
    }
    case TYPE_MUT_PTR:
    {
      type_names.push_back({});
      fmt::format_to(std::back_inserter(type_names.back()), "mutptr.{}", getTypeName(var.as<MutPtrType>()->getPointingTo()));
      return type_names.back();
    }
    case TYPE_OPTR:
      return "opaque_ptr";
    case TYPE_MUT_OPTR:
      return "mut_opaque_ptr";
      //TODO: add support for fn
    }
  }
  
  StringView TypeBuffer::getTypeName(TypeToken variant) const noexcept
  {
    return getTypeName(getType(variant));
  }
}