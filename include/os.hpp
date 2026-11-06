#pragma once

#include <string>

namespace os {

  namespace env {
    // Returns the value of the environment variable `var`.
    // If `var` does not exist, then return the fallback value `fallback`.
    std::string get(const char *var, const char *fallback = nullptr);

    // Returns a string representing `s` after environment variable expansion.
    // Environment variables are expanded similar to shell variables and begin
    // with a "$" and may be inclosed within "${" and "}".
    //   (e.g. "$HOME", "${HOME}" )
    //
    // A fallback value may be set if the environment variable does not exist
    // by adding a ":-" followed by the fallback value.
    //   (e.g. "{$HOME:-/home}" )
    //
    // Variables will also be expanded if found within the fallback value.
    //   (e.g. "${XDG_CONFIG_HOME:-$HOME/.config}" )
    //   (e.g. "${XDG_CONFIG_HOME:-${HOME:-/home}/.config}" )
    //
    // A valid environment variable identifier consists of alphanumeric
    // characters and the underscore ("_").
    //
    // Use the backslash ("\") to escape literal "$" and "}" characters.
    //   (e.g. "\\$HOME" -> "$HOME" )
    //   (e.g. "${VAR:-{\\}{\\}}" -> "{}{}" )
    std::string expand(const char *s);
  };

  inline int exec(std::vector<utl::string_view> args) {
    return ::execvp(args[0].data(), reinterpret_cast<char**>(args.data()));
  }

  inline pid_t fork() {
    return ::fork();
  }

  struct error {
    public:
      constexpr error() noexcept;
      constexpr error(int err) noexcept;

      std::string to_string() const noexcept;

    private:
      int errno_{};
  };

  struct file_descriptor {
    public:
      constexpr file_descriptor(int fd);

      ssize_t read(void *buf, size_t count) noexcept;
      ssize_t write(utl::string_view buf) noexcept;

    private:
      int fd_{-1};
  };

};

//*****************************************************************************
// struct os::error
//*****************************************************************************

constexpr inline os::error::error() noexcept {}
constexpr inline os::error::error(int err) noexcept : errno_{err} {}

inline std::string os::error::to_string() const noexcept {
  return ::strerror(errno_);
}

//*****************************************************************************
// struct os::file_descriptor
//*****************************************************************************

constexpr inline os::file_descriptor::file_descriptor(int fd) : fd_{fd} {}

inline ssize_t os::file_descriptor::read(void *buf, size_t count) noexcept {
  return ::read(fd_, buf, count);
}

inline ssize_t os::file_descriptor::write(utl::string_view buf) noexcept {
  return ::write(fd_, buf.data(), buf.length());
}
