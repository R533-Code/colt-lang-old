/*****************************************************************//**
 * @file   clt_dyncall.h
 * @brief  Wrapper over the Dyncall library.
 * Contains DynamicBinder, that allows to dynamically call a function
 * (at runtime).
 * This is useful for the interpreter to be able to call any C function
 * if it has its address.
 * This is inherently unsafe as no runtime checks can be done over
 * missing/invalid/extra arguments passed to a function.
 * 
 * @author RPC
 * @date   June 2024
 *********************************************************************/
#ifndef HG_CLT_DYNCALL
#define HG_CLT_DYNCALL

#include <dyncall/dyncall.h>
#include <common/assertions.h>
#include <common/types.h>
#include <run/qword_op.h>

namespace clt::run
{
  /// @brief Calls a function at runtime
  class DynamicBinder
  {
    /// @brief The DCCallVM responsible for generating the calls
    RAIIResource<DCCallVM, &dcFree> vm;

  public:
    /// @brief Constructor
    /// @param size The size to reserve
    DynamicBinder(size_t size = 4096) noexcept
      : vm(dcNewCallVM(size))
    {
      dcMode(vm.get(), DC_CALL_C_DEFAULT);
    }

    DynamicBinder(const DynamicBinder&) = delete;
    DynamicBinder(DynamicBinder&& other) noexcept = default;
    DynamicBinder& operator=(const DynamicBinder&) = delete;
    DynamicBinder& operator=(DynamicBinder&&) noexcept = default;

    /// @brief Adds a new argument to the next function call
    /// @tparam T The type of the value to add
    /// @param value The value to add
    template<typename T>
    void push_arg(T value) noexcept
    {
      if constexpr (std::same_as<T, char> || std::same_as<T, unsigned char> || std::same_as<T, signed char>)
        dcArgChar(vm.get(), (char)value);
      else if constexpr (std::same_as<T, bool>)
        dcArgBool(vm.get(), (bool)value);
      else if constexpr (std::same_as<T, short> || std::same_as<T, unsigned short>)
        dcArgShort(vm.get(), (short)value);
      else if constexpr (std::same_as<T, int> || std::same_as<T, unsigned>)
        dcArgInt(vm.get(), (int)value);
      else if constexpr (std::same_as<T, long int> || std::same_as<T, long unsigned>)
        dcArgLong(vm.get(), (long int)value);
      else if constexpr (std::same_as<T, long long int> || std::same_as<T, long long unsigned>)
        dcArgLongLong(vm.get(), (long long int)value);
      else if constexpr (std::same_as<T, float>)
        dcArgFloat(vm.get(), value);
      else if constexpr (std::same_as<T, double>)
        dcArgDouble(vm.get(), value);
      else if constexpr (std::is_pointer_v<T>)
        dcArgPointer(vm.get(), (void*)value);
      else
        clt::unreachable("Invalid type for push_arg!");
    }

    /// @brief Adds a new argument to the next function call
    /// @param qword The argument to add
    /// @param type The type of the argument
    void push_qword(QWORD_t qword, run::TypeOp type) noexcept
    {
      using enum clt::run::TypeOp;
      
      switch (type)
      {
      case i8_t:
      case u8_t:
        push_arg(qword.as<i8>());
        break;
      case i16_t:
      case u16_t:
        push_arg(qword.as<i16>());
        break;
      case i32_t:
      case u32_t:
        push_arg(qword.as<i32>());
        break;
      case i64_t:
      case u64_t:
        push_arg(qword.as<i64>());
        break;
      case f32_t:
        push_arg(qword.as<f32>());
        break;
      case f64_t:
        push_arg(qword.as<f64>());
        break;
      }
    }

    /// @brief Calls a function that returns value using the arguments
    /// pushed using 'push_arg/push_qword'.
    /// @tparam T The return type of the function
    /// @tparam ...Args The expected arguments types
    /// @param fn The function address
    /// @return The returned value of the function
    template<typename T, typename... Args>
    T call(T(*fn)(Args...)) noexcept
    {
      ON_SCOPE_EXIT{
        dcReset(vm.get());
      };
      if constexpr (std::same_as<T, char> || std::same_as<T, unsigned char> || std::same_as<T, signed char>)
        return (T)dcCallChar(vm.get(), (void*)fn);
      else if constexpr (std::same_as<T, bool>)
        return (T)dcCallBool(vm.get(), (void*)fn);
      else if constexpr (std::same_as<T, short> || std::same_as<T, unsigned short>)
        return (T)dcCallShort(vm.get(), (void*)fn);
      else if constexpr (std::same_as<T, int> || std::same_as<T, unsigned>)
        return (T)dcCallInt(vm.get(), (void*)fn);
      else if constexpr (std::same_as<T, long int> || std::same_as<T, long unsigned>)
        return (T)dcCallLong(vm.get(), (void*)fn);
      else if constexpr (std::same_as<T, long long int> || std::same_as<T, long long unsigned>)
        return (T)dcCallLongLong(vm.get(), (void*)fn);
      else if constexpr (std::same_as<T, float>)
        return (T)dcCallFloat(vm.get(), (void*)fn);
      else if constexpr (std::same_as<T, double>)
        return (T)dcCallDouble(vm.get(), (void*)fn);
      else if constexpr (std::is_pointer_v<T>)
        return (T)dcCallPointer(vm.get(), (void*)fn);
      else if constexpr (std::is_void_v<T>)
        return dcCallVoid(vm.get(), (void*)fn);
      else
        clt::unreachable("Invalid type for call_fn!");
    }

    /// @brief Calls a function that returns void using the arguments
    /// pushed using 'push_arg/push_qword'.
    /// @param fn The address of the function to call
    void call_fn(void* fn) noexcept
    {
      call((void(*)())fn);
    }

    /// @brief Calls a function that returns value using the arguments
    /// pushed using 'push_arg/push_qword'.
    /// @param fn The address of the function to call
    /// @param return_t The return type of the function
    /// @return QWORD that represents the return value
    QWORD_t call_fn(void* fn, run::TypeOp return_t) noexcept
    {      
      QWORD_t ret;
      switch (return_t)
      {
      case clt::run::TypeOp::i8_t:
      case clt::run::TypeOp::u8_t:
         ret.bit_assign(call((i8(*)())fn));
         break;
      case clt::run::TypeOp::i16_t:
      case clt::run::TypeOp::u16_t:
        ret.bit_assign(call((i16(*)())fn));
        break;
      case clt::run::TypeOp::i32_t:
      case clt::run::TypeOp::u32_t:
        ret.bit_assign(call((i32(*)())fn));
        break;
      case clt::run::TypeOp::i64_t:
      case clt::run::TypeOp::u64_t:
        ret.bit_assign(call((i64(*)())fn));
        break;
      case clt::run::TypeOp::f32_t:
        ret.bit_assign(call((f32(*)())fn));
        break;
      case clt::run::TypeOp::f64_t:
        ret.bit_assign(call((f64(*)())fn));
        break;
      }
    }
  };
}

#endif // !HG_CLT_DYNCALL