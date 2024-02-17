//=================================================================================================
// Copyright (C) 2023 GRAPE Contributors
// MIT License
//=================================================================================================

#pragma once

#include <exception>
#include <source_location>
#include <string>

#include "grape/utils/stacktrace.h"

namespace grape {

//=================================================================================================
/// Abstract interface for exceptions
class AbstractException {
public:
  /// @return Non-modifiable reference to error message
  [[nodiscard]] virtual auto what() const noexcept -> const std::string& = 0;

  /// @return Location in the source where the exception occurred
  [[nodiscard]] virtual auto where() const noexcept -> const std::source_location& = 0;

  /// @return Backtrace leading up to exception
  [[nodiscard]] virtual auto when() const noexcept -> const utils::StackTrace& = 0;

  /// A utility method (Lippincott function) that consumes all exceptions and prints diagnostics
  /// except custom data set by user in the derived classes
  static void consume() noexcept;

  virtual ~AbstractException() = default;
};

//=================================================================================================
/// Exception class, inspired from Peter Muldoon (https://youtu.be/Oy-VTqz1_58)
///
/// Guidelines:
/// - Use exceptions only for serious/infrequent/unexpected errors where getting out of control
/// flow to prevent further damage is more important than trying to continue onward.
///   - For shallow returns, prefer 'std::optional' and 'std::expected'
///   - Define as few exception types as possible
///   - Exceptions are defined by their catch handler usage. Use them for:
///     - Error tracing and logging
///     - Stack unwinding for terminally fatal or transactionally fatal errors
/// - The following NOT good uses for exceptions:
///   - Resource management (eg: to release resources in catch block). Use RAII instead.
///   - Loop control flow. Use return status codes instead.
///   - Memory corruption or exhaustion. Just terminate instead.
/// @todo Integrate std::stacktrace when compiler support becomes available
/// @include exception_example.cpp
template <typename DATA_T>
class Exception : public AbstractException {
public:
  /// @param message A context-specific description of the error
  /// @param data Contextual user-defined data
  /// @param location Location in the source where the error was triggered at
  Exception(std::string message, DATA_T data, std::source_location location,
            utils::StackTrace trace)
    : message_{ std::move(message) }
    , data_{ std::move(data) }
    , location_{ location }
    , backtrace_{ std::move(trace) } {
  }

  [[nodiscard]] auto what() const noexcept -> const std::string& override {
    return message_;
  }

  [[nodiscard]] auto where() const noexcept -> const std::source_location& override {
    return location_;
  }

  /// @return Backtrace leading up to exception
  [[nodiscard]] auto when() const noexcept -> const utils::StackTrace& override {
    return backtrace_;
  }

  /// @return User-defined contextual data
  [[nodiscard]] auto data() const noexcept -> const DATA_T& {
    return data_;
  }

private:
  std::string message_;
  DATA_T data_;
  std::source_location location_;
  utils::StackTrace backtrace_;
};

//=================================================================================================
/// Errors from system calls or platform library functions
struct SystemError {
  int code;  //!< error code (errno) set by failing system call or library function
  std::string_view function_name;  //!< name of system call or library function
};

using SystemException = grape::Exception<SystemError>;

/// User function to throw an exception derived from Exception<DATA_T>
template <typename T, class DATA_T>
  requires std::derived_from<T, Exception<DATA_T>>
constexpr void panic(std::string message, DATA_T data,                                 //
                     std::source_location location = std::source_location::current(),  //
                     utils::StackTrace trace = utils::StackTrace::current()) {
  throw T{ message, data, location, trace };
}

}  // namespace grape
