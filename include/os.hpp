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

};
