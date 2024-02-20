/*****************************************************************//**
 * @file   error_reporter.h
 * @brief  Contains composable reporters.
 * 
 * @author RPC
 * @date   January 2024
 *********************************************************************/
#ifndef HG_COLT_ERROR_REPORTER
#define HG_COLT_ERROR_REPORTER

#include "structs/string.h"
#include "structs/unique_ptr.h"
#include "structs/option.h"
#include "io_reporter.h"

namespace clt::lng
{
  template<typename T, typename... Args>
  /// @brief A Reporter is any type supporting message(...), error(...), warn(...) methods
  concept Reporter = requires (T reporter, StringView str, const Option<SourceInfo>& src_info, const Option<ReportNumber>& msg_nb)
  {
    { reporter.message(str, src_info, msg_nb) } -> std::same_as<void>;
    { reporter.error(str, src_info, msg_nb)   } -> std::same_as<void>;
    { reporter.warn(str, src_info, msg_nb)    } -> std::same_as<void>;
  };

  /// @brief Base class for all error reporting mechanism
  class ErrorReporter
  {
  public:
    /// @brief Reports a message
    /// @param str The message string
    /// @param src_info The source information
    /// @param msg_nb The message number
    virtual void message(StringView str, const Option<SourceInfo>& src_info = None, const Option<ReportNumber>& msg_nb = None) noexcept = 0;
    /// @brief Reports a warning
    /// @param str The warning string
    /// @param src_info The source information
    /// @param msg_nb The warning number
    virtual void    warn(StringView str, const Option<SourceInfo>& src_info = None, const Option<ReportNumber>& msg_nb = None) noexcept = 0;
    /// @brief Reports an error
    /// @param str The error string
    /// @param src_info The source information
    /// @param msg_nb The error number
    virtual void   error(StringView str, const Option<SourceInfo>& src_info = None, const Option<ReportNumber>& msg_nb = None) noexcept = 0;
    
    /// @brief Destructor
    virtual ~ErrorReporter() noexcept {};
  };

  namespace details
  {
    template<Reporter Rep>
    /// @brief Helper to convert a 'Reporter' to an ErrorReporter
    struct ToErrorReporter
      : public ErrorReporter, public Rep
    {
      template<typename... Args>
      constexpr ToErrorReporter(Args&&... args) noexcept(std::is_nothrow_constructible_v<Rep, Args...>)
        : Rep(std::forward<Args>(args)...) {}

      void message(StringView str, const Option<SourceInfo>& src_info = None, const Option<ReportNumber>& msg_nb = None) noexcept override
      {
        Rep::message(str, src_info, msg_nb);
      }

      void warn(StringView str, const Option<SourceInfo>& src_info = None, const Option<ReportNumber>& msg_nb = None) noexcept override
      {
        Rep::warn(str, src_info, msg_nb);
      }

      void error(StringView str, const Option<SourceInfo>& src_info = None, const Option<ReportNumber>& msg_nb = None) noexcept override
      {
        Rep::error(str, src_info, msg_nb);
      }

      ~ToErrorReporter() override {};
    };
  }  

  template<Reporter Rep, auto ALLOCATOR = mem::GlobalAllocatorDescription, typename... Args>
  UniquePtr<ErrorReporter> make_error_reporter(Args&&... args) noexcept(std::is_nothrow_constructible_v<Rep, Args...>)
  {
    return make_unique<details::ToErrorReporter<Rep>>(std::forward<Args>(args)...);
  }  
}

#endif // !HG_COLT_ERROR_REPORTER
