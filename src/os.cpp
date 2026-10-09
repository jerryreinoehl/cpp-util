#include "os.hpp"
#include "util.hpp"

#include <sstream>

#include <iostream>  // TODO

//*****************************************************************************
// os::env
//*****************************************************************************

std::string os::env::get(const char *var, const char *fallback) {
  const char *c = std::getenv(var);
  if (c == nullptr) {
    c = fallback;
  }
  return c;
}

std::string os::env::expand(const char *s) {
  char *env;
  std::stringstream expanded{};

  bool is_braced_var{false};
  bool scan_to_end_of_braced_var{false};
  bool suppress_increment{false};
  int depth{0};

  utl::const_char_iterator it{s};
  utl::const_char_iterator end{};
  for (; it; suppress_increment || ++it, suppress_increment = false) {
    if (scan_to_end_of_braced_var) {
      std::cout << "in ignore\n";
      int count{1};
      scan_to_end_of_braced_var = false;
      for (; it && count > 0; ++it) {
        std::cout << "[ig] *it = " << *it << '\n';
        if (*it == '{') {
          ++count;
        } else if (*it == '}') {
          --count;
          if (count == 0) break;
        } else if (*it == '\\') {
          ++it;
          continue;
        }
      }
      --depth;
    } else if (*it == '$') {
      std::cout << "\nin variable expansion\n";

      ++it;
      if (*it == '{') {
        is_braced_var = true;
        continue;
      }

      end = it;
      end.seek([] (char c) { return isalnum(c) || c == '_'; });
      std::cout << "*end: " << *end << '\n';
      std::cout << "${} it.to_string(end) = " << it.to_string(end) << '\n';
      std::cout << "scan_to_end_of_braced_var = " << scan_to_end_of_braced_var << '\n';
      std::cout << ">depth: " << depth << '\n';

      env = std::getenv(it.to_string(end).c_str());
      it = end;
      suppress_increment = true;

      if (env != nullptr) {
        expanded << env;
        continue;
      }
    } else if (is_braced_var) {
      std::cout << "\nin braced variable expansion\n";
      is_braced_var = false;
      ++depth;

      end = it;
      end.seek([] (char c) { return isalnum(c) || c == '_'; });
      std::cout << "*end: " << *end << '\n';
      std::cout << "${} it.to_string(end) = " << it.to_string(end) << '\n';
      std::cout << "scan_to_end_of_braced_var = " << scan_to_end_of_braced_var << '\n';
      std::cout << ">depth: " << depth << '\n';

      env = std::getenv(it.to_string(end).c_str());

      it = end;
      if (env != nullptr) {
        scan_to_end_of_braced_var = true;
        suppress_increment = true;
        expanded << env;
        continue;
      }

      if (*it == '}') {
        --depth;
        continue;
      }

      if (!it.starts_with(":-")) {
        continue;
      }

      it.seek(":-");
      suppress_increment = true;
    }
    else if (*it == '}' and depth > 0) {
      --depth;
    } else if (*it == '\\') {
      ++it;
      expanded << *it;
    } else {
      std::cout << "<< " << *it << '\n';
      expanded << *it;
    }
  }

  return expanded.str();
}
