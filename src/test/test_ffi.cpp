/*****************************************************************//**
 * @file   test_ffi.cpp
 * @brief  Contains the implementation of 'test_ffi'.
 * 
 * @author RPC
 * @date   June 2024
 *********************************************************************/
#include "test_ffi.h"

#define TEST_IDENTITY(type, value) do { binder.push_arg((type)value); \
if (binder.call(+[](decltype(value) a) { return a; }) != (type)value) { \
  ++error_count, \
  io::print_error("FFI does not work for '" #type "'."); }\
} while (false)

extern "C" void CLT_NOP()
{
  // This method doesn't do anything except provide an address for 'test_ffi'.
};

namespace clt::test
{

  void test_ffi(u32& error_count) noexcept
  {
    using namespace run;
    
    io::print_message("Testing FFI...");
    DynamicBinder binder;   
    TEST_IDENTITY(char, 'a');
    TEST_IDENTITY(bool, true);
    TEST_IDENTITY(bool, false);

    TEST_IDENTITY(i8, std::numeric_limits<i8>::min());
    TEST_IDENTITY(u8, std::numeric_limits<u8>::max());
    
    TEST_IDENTITY(i16, std::numeric_limits<i16>::min());
    TEST_IDENTITY(u16, std::numeric_limits<u16>::max());
    
    TEST_IDENTITY(i32, std::numeric_limits<i32>::min());
    TEST_IDENTITY(u32, std::numeric_limits<u32>::max());
    
    TEST_IDENTITY(i64, std::numeric_limits<i64>::min());
    TEST_IDENTITY(u64, std::numeric_limits<u64>::min());
    
    TEST_IDENTITY(void(*)(), &CLT_NOP);
    
    TEST_IDENTITY(f32, -0.24f);
    TEST_IDENTITY(f64, -24e30);
  }
}

#undef TEST_IDENTITY