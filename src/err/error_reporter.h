#ifndef HG_COLT_ERROR_REPORTER
#define HG_COLT_ERROR_REPORTER

#include "structs/string.h"
#include "io_reporter.h"

namespace clt::lng
{
  /// @brief Base class for all error reporting mechanism
  class ErrorReporter
  {
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
    virtual ~ErrorReporter() noexcept = 0;
  };

  class LimiterReporter
  {

  };
}

#endif // !HG_COLT_ERROR_REPORTER
