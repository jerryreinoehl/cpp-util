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
      constexpr file_descriptor() noexcept;
      constexpr file_descriptor(int fd) noexcept;

      // Reads `count` bytes from this file descriptor into `buf`.
      // Returns the number of bytes read or an `os::error`.
      utl::expected<size_t, os::error> read(void *buf, size_t count) noexcept;

      // Reads all of the output from this file descriptor as a `std::string`.
      utl::expected<std::string, os::error> read() noexcept;

      // Write the `buf` to this file descriptor.
      // Returns the number of bytes written or an `os::error`.
      utl::expected<size_t, os::error> write(utl::string_view buf) noexcept;

      // Closes this file descriptor.
      // Return 0 on success or an `os::error`.
      //
      // TODO: Add impl for utl::expected<void, E>.
      utl::expected<int, os::error> close() noexcept;

      // Write formatted string to this file descriptor.
      template <typename... Args>
      utl::expected<size_t, os::error> printf(utl::string_view format, Args... args) noexcept;

      // `file_descriptor` evaluates to `true` if this is a valid file descriptor.
      explicit constexpr operator bool() const noexcept;

      // Returns the file descriptor number.
      int fd() const noexcept;

    private:
      int fd_{-1};
  };

  utl::expected<pid_t, os::error> fork() noexcept;

  utl::expected<os::file_descriptor, os::error> dup(os::file_descriptor oldfd, os::file_descriptor newfd) noexcept;

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

constexpr inline os::file_descriptor::file_descriptor() noexcept {}

constexpr inline os::file_descriptor::file_descriptor(int fd) noexcept : fd_{fd} {}

inline utl::expected<size_t, os::error> os::file_descriptor::read(void *buf, size_t count) noexcept {
  ssize_t rc = ::read(fd_, buf, count);
  if (rc < 0) {
    return utl::unexpected<os::error>{errno};
  }
  return static_cast<size_t>(rc);
}

inline utl::expected<size_t, os::error> os::file_descriptor::write(utl::string_view buf) noexcept {
  ssize_t rc = ::write(fd_, buf.data(), buf.length());
  if (rc < 0) {
    return utl::unexpected<os::error>{errno};
  }
  return static_cast<size_t>(rc);
}

inline utl::expected<int, os::error> os::file_descriptor::close() noexcept {
  int rc = ::close(fd_);
  if (rc < 0) {
    return utl::unexpected<os::error>{errno};
  }
  return 0;
}

template <typename... Args>
inline utl::expected<size_t, os::error> os::file_descriptor::printf(utl::string_view format, Args... args) noexcept {
  int rc = ::dprintf(fd_, format.data(), args...);
  if (rc < 0) {
    return utl::unexpected<os::error>{errno};
  }
  return static_cast<size_t>(rc);
}

constexpr inline os::file_descriptor::operator bool() const noexcept {
  return fd_ >= 0;
}

inline int os::file_descriptor::fd() const noexcept {
  return fd_;
}

//*****************************************************************************

inline utl::expected<pid_t, os::error> os::fork() noexcept {
  pid_t rc = ::fork();
  if (rc < 0) {
    return utl::unexpected<os::error>{errno};
  }
  return rc;
}

inline utl::expected<os::file_descriptor, os::error>
os::dup(os::file_descriptor oldfd, os::file_descriptor newfd) noexcept {
  int rc = ::dup2(oldfd.fd(), newfd.fd());
  if (rc < 0) {
    return utl::unexpected<os::error>{errno};
  }
  return os::file_descriptor{rc};
}
