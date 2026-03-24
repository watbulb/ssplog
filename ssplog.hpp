#pragma once
#if __cplusplus < 202302L
#error C++ 23 required
#endif

#ifdef __clang__
#define no_inline [[clang::noinline]]
#elifdef _MSC_VER
#define no_inline [[msvc::noinline]]
#elifdef __GNUC__
#define no_inline [[gnu::noinline]]
#else
#error Unsupported Compiler
#endif

#include <concepts>
#include <type_traits>

#include <chrono>
#include <functional>
#include <iomanip>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <initializer_list>
#include <mutex>


enum class log_level { debug, info, warn, error };


namespace ssp::log::detail
{

  template <typename T>
  concept is_string_trait = requires(T t) {
    { t.as_string() } -> std::same_as<std::string>;
  };

  template <typename T>
  concept serializable = (
    std::is_convertible_v<T, std::string> || is_string_trait<T> ||
    std::is_arithmetic_v<T> ||
    std::is_same_v<T, std::chrono::system_clock::time_point> ||
    std::is_same_v<T, log_level>
  );

  template <serializable T>
  constexpr const std::basic_string<char> serialize(const T &value) noexcept
  {
    if constexpr(std::is_convertible_v<T, std::string>) {
      return value;
    } else if constexpr(is_string_trait<T>) {
      return value.as_string();
    } else if constexpr(std::is_arithmetic_v<T>) {
      return std::to_string(value);
    } else if constexpr(std::is_same_v<T, std::chrono::system_clock::time_point>) {
      const auto time_t = std::chrono::system_clock::to_time_t(value);
      std::basic_string<char> time_s = std::asctime(std::gmtime(&time_t));
      time_s.pop_back();
      return time_s;
    } else if constexpr(std::is_same_v<T, log_level>) {
        switch (value) {
          case log_level::debug: return "debug";
          case log_level::info:  return "info";
          case log_level::warn:  return "warn";
          case log_level::error: return "error";
        }
    } else {
      return "";
    }
  }


} // namespace ssplog::detail

namespace ssp::log {

struct log_field {
  const std::string &key;
  const std::string value;

  template<detail::serializable T>
  constexpr log_field(const std::string &k, const T &v)
    : key{k}, value{detail::serialize(v)}
    {};
};
struct log_fields : std::vector<log_field> {
  const std::vector<log_field> fields;

  template <detail::serializable T>
  constexpr log_fields(const std::string &k, const T &v)
    : fields{{detail::serialize(k), detail::serialize(v)}}
    {};

  template<detail::serializable K = const std::string &, detail::serializable V = const std::string &>
  constexpr log_fields(const std::initializer_list<log_field> &fs)
    : fields{fs}
    {}

  constexpr log_fields(const log_field &f)
    : fields{f}
    {};
};

/// Logger

template <
  typename clock_type   = const std::chrono::system_clock,
  typename printer_type = const std::function<void(const std::string &)>
>
class Logger
{
public:
  constexpr Logger(
    const printer_type printer,
    const log_fields   globals = {}
  ) : m_printer{printer},
      m_globals{globals},
      m_printerlock{}
    { }

  no_inline void  info(const std::string &msg, const log_fields &fields = {}) { log(log_level::info,  msg, fields); }
  no_inline void  warn(const std::string &msg, const log_fields &fields = {}) { log(log_level::warn,  msg, fields); }
  no_inline void debug(const std::string &msg, const log_fields &fields = {}) { log(log_level::debug, msg, fields); }
  no_inline void error(const std::string &msg, const log_fields &fields = {}) { log(log_level::error, msg, fields); }

  no_inline void log(
    const log_level   &lvl,
    const std::string &msg,
    const log_fields  &fields = {}
  ) noexcept
  {
    static std::ostringstream ostream{};
    {
      std::scoped_lock lock{m_printerlock};
      ostream << "time="    << '[' << detail::serialize(clock_type::now()) << ']' << ' ';
      ostream << "level="   << '[' << detail::serialize(lvl) << ']' <<  ' ';
      ostream << "message=" << std::quoted(msg);
      for (const auto &[key, value] : m_globals) {
        ostream << " " << key << "=" << value;
      }
      for (const auto &[key, value] : fields) {
        ostream << " " << key << "=" << value;
      }
      m_printer(std::move(ostream.str()));
      ostream.str("");
    }
  }

private:
  const printer_type m_printer;
  const log_fields   m_globals;
  std::mutex m_printerlock;
}; // Logger
}  // namespace ssp::log

