/*****************************************************************//**
 * @file   composable_reporter.h
 * @brief  Contains the building blocks of ErrorReporter.
 * 
 * @author RPC
 * @date   January 2024
 *********************************************************************/
#ifndef HG_COLT_COMPOSABLE_REPORTER
#define HG_COLT_COMPOSABLE_REPORTER

#include "error_reporter.h"

namespace clt::lng
{
  /// @brief Filter function for FilterReporter (true to keep, false to remove)
  using filter_reporter_t = bool(*)(StringView, const Option<SourceInfo>&, const Option<ReportNumber>&) noexcept;

  /// @brief Consumes all reports
  struct SinkReporter
  {
    /// @brief Does nothing
    void message(StringView, const Option<SourceInfo>&, const Option<ReportNumber>&) const noexcept
    {
      // Does nothing
    }

    /// @brief Does nothing
    void warn(StringView, const Option<SourceInfo>&, const Option<ReportNumber>&) const noexcept
    {
      // Does nothing
    }

    /// @brief Does nothing
    void error(StringView, const Option<SourceInfo>&, const Option<ReportNumber>&) const noexcept
    {
      // Does nothing
    }
  };

  /// @brief Prints the reports to the console
  struct ConsoleReporter
  {
    /// @brief Prints the message to the console
    /// @param str The message
    /// @param info The source information if it exist
    /// @param nb The report information if it exist
    void message(StringView str, const Option<SourceInfo>& info, const Option<ReportNumber>& nb) const noexcept
    {
      generate_message(str, info, nb);
    }

    /// @brief Prints the warning to the console
    /// @param str The warning
    /// @param info The source information if it exist
    /// @param nb The report information if it exist
    void warn(StringView str, const Option<SourceInfo>& info, const Option<ReportNumber>& nb) const noexcept
    {
      generate_warn(str, info, nb);
    }

    /// @brief Prints the error to the console
    /// @param str The error
    /// @param info The source information if it exist
    /// @param nb The report information if it exist
    void error(StringView str, const Option<SourceInfo>& info, const Option<ReportNumber>& nb) const noexcept
    {
      generate_error(str, info, nb);
    }
  };

  template<Reporter Rep>
  /// @brief Filters reports generated
  /// @tparam Rep The reporter to forward reports to if not filtered
  class FilterReporter
    : public Rep
  {
    /// @brief Message filter or nullptr
    filter_reporter_t message_filter;
    /// @brief Warn filter or nullptr
    filter_reporter_t warn_filter;
    /// @brief Error filter or nullptr
    filter_reporter_t error_filter;

  public:
    FilterReporter() = delete;
    constexpr FilterReporter(FilterReporter&&)      noexcept = default;
    constexpr FilterReporter(const FilterReporter&) noexcept = default;

    template<typename... Args>
    /// @brief Constructor
    /// @param err The error filter (or nullptr)
    /// @param wrn The warning filter (or nullptr)
    /// @param msg The message filter (or nullptr)
    /// @param args Arguments to forward to the constructor of 'Rep'
    constexpr FilterReporter(filter_reporter_t err, filter_reporter_t wrn, filter_reporter_t msg, Args&&... args) noexcept(std::is_nothrow_constructible_v<Rep, Args...>)
      : Rep(std::forward<Args>(args)...), message_filter(msg), warn_filter(wrn), error_filter(err) {}

    /// @brief Forward the message to 'Rep' if there is no message filter
    ///        or the filter returns true.
    /// @param str The message
    /// @param src_info The source information if it exist
    /// @param msg_nb The report information if it exist
    void message(StringView str, const Option<SourceInfo>& src_info = None, const Option<ReportNumber>& msg_nb = None) noexcept
    {
      if (message_filter == nullptr || message_filter(str, src_info, msg_nb))
        Rep::message(str, src_info, msg_nb);
    }

    /// @brief Forward the warning to 'Rep' if there is no warning filter
    ///        or the filter returns true.
    /// @param str The warning
    /// @param src_info The source information if it exist
    /// @param msg_nb The report information if it exist
    void warn(StringView str, const Option<SourceInfo>& src_info = None, const Option<ReportNumber>& msg_nb = None) noexcept
    {
      if (warn_filter == nullptr || warn_filter(str, src_info, msg_nb))
        Rep::warn(str, src_info, msg_nb);
    }

    /// @brief Forward the error to 'Rep' if there is no error filter
    ///        or the filter returns true.
    /// @param str The error
    /// @param src_info The source information if it exist
    /// @param msg_nb The report information if it exist
    void error(StringView str, const Option<SourceInfo>& src_info = None, const Option<ReportNumber>& msg_nb = None) noexcept
    {
      if (error_filter == nullptr || error_filter(str, src_info, msg_nb))
        Rep::error(str, src_info, msg_nb);
    }
  };

  template<Reporter Rep>
  /// @brief Limits the number reports generated
  /// @tparam Rep The reporter to forward reports to if the limit is not hit
  class LimiterReporter
    : public Rep
  {
    /// @brief Count of remaining messages
    u16 message_rem;
    /// @brief Count of remaining warnings
    u16 warn_rem;
    /// @brief Count of remaining errors
    u16 error_rem;
    /// @brief True if the LimiterReporter will no longer forward messages to 'Rep'
    bool exhausted_message : 1 = false;
    /// @brief True if the LimiterReporter will no longer forward warnings to 'Rep'
    bool exhausted_warn : 1 = false;
    /// @brief True if the LimiterReporter will no longer forward errors to 'Rep'
    bool exhausted_error : 1 = false;

    /// @brief Special value that will not be decremented (useful to not limit a category)
    static constexpr u16 NO_DECREMENT = std::numeric_limits<u16>::max();

  public:
    LimiterReporter() = delete;
    constexpr LimiterReporter(LimiterReporter&&)      noexcept = default;
    constexpr LimiterReporter(const LimiterReporter&) noexcept = default;

    template<typename... Args>
    /// @brief Constructor
    /// @param err The error limit or none if no limit on errors
    /// @param wrn The warning limit or none if no limit on warnings
    /// @param msg The message limit or none if no limit on messages
    /// @param args Arguments to forward to the constructor of 'Rep'
    /// @pre err.value() and wrn.value() and msg.value() != 0
    constexpr LimiterReporter(Option<u16> err, Option<u16> wrn, Option<u16> msg, Args&&... args) noexcept(std::is_nothrow_constructible_v<Rep, Args...>)
      : Rep(std::forward<Args>(args)...)
      , message_rem(msg.is_value() ? *msg : NO_DECREMENT)
      , warn_rem(wrn.is_value() ? *wrn : NO_DECREMENT)
      , error_rem(err.is_value() ? *err : NO_DECREMENT)
    {
      assert_true("Invalid arguments for LimiterReporter!",
        message_rem != 0, warn_rem != 0, error_rem != 0);
    }

    /// @brief Forward the message to 'Rep' if the message limit was not hit.
    /// When the limit is hit, forwards "No more messages will be reported." once,
    /// and stop reporting messages.
    /// @param str The message
    /// @param src_info The source information if it exist
    /// @param msg_nb The report information if it exist
    void message(StringView str, const Option<SourceInfo>& src_info = None, const Option<ReportNumber>& msg_nb = None) noexcept
    {
      if (message_rem == 0 && exhausted_message)
        return;
      if ((message_rem -= static_cast<u16>(message_rem != NO_DECREMENT)))
        return Rep::message(str, src_info, msg_nb);
      exhausted_message = true;
      Rep::message("No more messages will be reported.", None, None);
    }

    /// @brief Forward the warning to 'Rep' if the warning limit was not hit.
    /// When the limit is hit, forwards "No more warnings will be reported." once,
    /// and stop reporting warnings.
    /// @param str The warning
    /// @param src_info The source information if it exist
    /// @param msg_nb The report information if it exist
    void warn(StringView str, const Option<SourceInfo>& src_info = None, const Option<ReportNumber>& msg_nb = None) noexcept
    {
      if (warn_rem == 0 && exhausted_warn)
        return;
      if ((warn_rem -= static_cast<u16>(warn_rem != NO_DECREMENT)))
        return Rep::warn(str, src_info, msg_nb);
      exhausted_warn = true;
      Rep::warn("No more warnings will be reported.", None, None);
    }

    /// @brief Forward the warning to 'Rep' if the warning limit was not hit.
    /// When the limit is hit, forwards "No more warnings will be reported." once,
    /// and stop reporting warnings.
    /// @param str The warning
    /// @param src_info The source information if it exist
    /// @param msg_nb The report information if it exist
    void error(StringView str, const Option<SourceInfo>& src_info = None, const Option<ReportNumber>& msg_nb = None) noexcept
    {
      if (error_rem == 0 && exhausted_error)
        return;
      if ((error_rem -= static_cast<u16>(error_rem != NO_DECREMENT)))
        return Rep::error(str, src_info, msg_nb);
      exhausted_error = true;
      Rep::error("No more errors will be reported.", None, None);
    }
  };
}

#endif // !HG_COLT_COMPOSABLE_REPORTER
