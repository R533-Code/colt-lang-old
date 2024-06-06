#ifndef HG_CLT_DYNCALL
#define HG_CLT_DYNCALL

#include <dyncall/dyncall.h>
#include <util/assertions.h>
#include <util/types.h>
#include <run/qword_op.h>
#include <util/raii_helper.h>

namespace clt::run
{
  /// @brief Calls a function at runtime
  class DynamicBinder
  {
    RAIIResource<DCCallVM, &dcFree> vm;

  public:
    DynamicBinder(size_t size = 4096) noexcept
      : vm(dcNewCallVM(size)) {}

    DynamicBinder(const DynamicBinder&) = delete;
    DynamicBinder(DynamicBinder&& other) noexcept = default;
    DynamicBinder& operator=(const DynamicBinder&) = delete;
    DynamicBinder& operator=(DynamicBinder&&) noexcept = default;

    template<typename T>
    void push_arg(T value) const noexcept
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

    template<typename T, typename... Args>
    T call_fn(T(*fn)(Args...)) noexcept
    {
      ON_SCOPE_EXIT{
        dcReset(vm.get());
      };
      if constexpr (std::same_as<T, char> || std::same_as<T, unsigned char> || std::same_as<T, signed char>)
        return (T)dcCallChar(vm.get(), fn);
      else if constexpr (std::same_as<T, bool>)
        return (T)dcCallBool(vm.get(), fn);
      else if constexpr (std::same_as<T, short> || std::same_as<T, unsigned short>)
        return (T)dcCallShort(vm.get(), fn);
      else if constexpr (std::same_as<T, int> || std::same_as<T, unsigned>)
        return (T)dcCallInt(vm.get(), fn);
      else if constexpr (std::same_as<T, long int> || std::same_as<T, long unsigned>)
        return (T)dcCallLong(vm.get(), fn);
      else if constexpr (std::same_as<T, long long int> || std::same_as<T, long long unsigned>)
        return (T)dcCallLongLong(vm.get(), fn);
      else if constexpr (std::same_as<T, float>)
        return (T)dcCallFloat(vm.get(), fn);
      else if constexpr (std::same_as<T, double>)
        return (T)dcCallDouble(vm.get(), fn);
      else if constexpr (std::is_pointer_v<T>)
        return (T)dcCallPointer(vm.get(), fn);
      else if constexpr (std::is_void_v<T>)
        return dcCallVoid(vm.get(), fn);
      else
        clt::unreachable("Invalid type for call_fn!");
    }
  };
}

#endif // !HG_CLT_DYNCALL